/**
 * @file main.cpp
 * @brief RZ/V2H DRP-AI Sample Application for Lightnet YOLOv2 with MIPI/USB Camera
 *
 * @copyright Copyright (c) 2023 Renesas Electronics Corporation. All rights reserved.
 * @copyright Copyright (c) 2024 IMD Technologies Ltd.
 */

#include <linux/drpai.h>

#include <array>
#include <chrono>
#include <cstdlib>
#include <deque>
#include <iostream>
#include <iomanip> // Include for std::setprecision

#include <filesystem>
#include <mutex>

#include <argparse/argparse.hpp>
#include <nlohmann/json.hpp>

#include "box.hpp"
#include "camera.hpp"
#include "define.h"
#include "define_color.h"
#include "dma_buffer.hpp"
#include "drp_ai_accelerator.hpp"
#include "image.hpp"
// clang-format off
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/spdlog.h"
#include "spdlog/fmt/chrono.h"
// clang-format on
#include "timer.hpp"

#include <linux/i2c-dev.h>
#include <i2c/smbus.h>
#include <cmath> // For trigonometric functions
#include <opencv2/opencv.hpp>        // Core OpenCV functionalities (Mat, Rect, Scalar, etc.)
#include <opencv2/imgproc.hpp>      // Image processing (e.g., cv::mean, cv::cvtColor)

//Robot Control Class
#include "robot_control.hpp"
//Move To target Class
#include "move_to_target.hpp"

//display class
#include "wayland.hpp"


// These are your two GPIO lines:
static const std::string BUTTON1_GPIO = "P5_5"; // or "gpio461"
static const std::string BUTTON2_GPIO = "P5_6"; // or "gpio462"
std::atomic<bool> gShouldTerminate(false);
static Wayland wayland;
using namespace std::chrono_literals;

/**
 * Constants
 */

/// @brief Thread sleep time
static constexpr auto kThreadSleepTime{1ms};

/// @brief The expected value for the "terminate request"s semaphore
static constexpr unsigned int kExpectedSemaphoreValue{1};

/*
 * Global Variables
 */
std::atomic<bool> robot_is_moving(false);

/**
 * @brief Used to notify all the threads that they should terminate
 * @details On startup, the semaphore is initialised to "1". Each thread tests the semaphore at the start of its loop,
 * and if the value has changed the thread terminates.
 */
static sem_t terminate_req_sem;

/// @brief Mutex to protect against simultaneous access to the list of detections
static std::mutex detections_mutex;

/// Thread synchronisation

static std::atomic<uint8_t> inference_start(0);
static std::atomic<uint8_t> img_obj_ready(0);
static std::atomic<uint8_t> hdmi_obj_ready(0);
static std::atomic<uint8_t> server_connected{0};

static std::atomic<size_t> active_buffer_index;

/**
 * Types
 */

struct Statistics {
  using SharedPtr = std::shared_ptr<Statistics>;

  double inference_time_ms{0};
  double post_processing_time_ms{0};
  double average_inference_frame_rate{0};
  double average_capture_frame_rate{0};
};

/*
 * Static functions
 */

/**
 * @brief Checks the semaphore to see if termination has been requested
 * @return True if the thread should terminate
 */
static bool terminationRequested() {
  int val = 0;
  if (0 != sem_getvalue(&terminate_req_sem, &val)) {
    std::cerr << "[ERROR] Failed to get semaphore Value" << std::endl;
    return true;
  }

  return (val != kExpectedSemaphoreValue);
}

// Reads the sysfs GPIO value file and returns true if "1", false if "0" (or on error).
bool readGpioValue(const std::string& gpio_name)
{
    // Build the path: e.g. "/sys/class/gpio/gpio461/value" 
    // or "/sys/class/gpio/P5_5/value" if your kernel creates that folder name.
    std::string path = "/sys/class/gpio/" + gpio_name + "/value";

    std::ifstream gpio_val_file(path);
    if (!gpio_val_file.is_open()) {
        std::cerr << "[ERROR] Cannot open " << path << std::endl;
        return false;
    }

    int val = 0;
    gpio_val_file >> val;  // Read integer from the file
    gpio_val_file.close();

    return (val == 1);
}

// An atomic flag you use to check for termination
extern std::atomic<bool> gShouldTerminate;

void GpioButtonThread()
{
   std::cout << "[INFO] GpioButtonThread started." << std::endl;

    // Store the initial (previous) states
    bool button1_prev = readGpioValue(BUTTON1_GPIO);
    bool button2_prev = readGpioValue(BUTTON2_GPIO);

    while (!gShouldTerminate.load()) 
    {
        bool button1_curr = readGpioValue(BUTTON1_GPIO);
        bool button2_curr = readGpioValue(BUTTON2_GPIO);

        // Detect a rising edge for Button1: 0 -> 1
        if (!button1_prev && button1_curr) {
            std::cout << "Button1 (GPIO " << BUTTON1_GPIO << ") pressed!\n";
            // Do something (only once per actual press)
        }

        // Detect a rising edge for Button2: 0 -> 1
        if (!button2_prev && button2_curr) {
            std::cout << "Button2 (GPIO " << BUTTON2_GPIO << ") pressed!\n";
			sRobotControl->clearQueue();
			sRobotControl->initUART();
            // Do something else(only once per actual press)
        }

        // Update previous states
        button1_prev = button1_curr;
        button2_prev = button2_curr;

        // Poll every 50 ms
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    std::cout << "[INFO] GpioButtonThread terminated." << std::endl;
}


/*****************************************
*Function Name: degreestoADC
 * Description   : Function to calculate the adc value from an angle
 * Arguments     : angle is the value in degres of the angle
 * Return value  : ADC value 
*****************************************/
int degreesToADC(double angle) {
    return static_cast<int>((2200.0 / 180.0) * angle + 900.0);
}

/*****************************************
*Function Name: calculateDistance
 * Description   : Function to calculate distance from camera to object
 * Arguments     : boxWith in pixels of object to calculate
 *		   boxHeight in pixels of object to calculate 
 * Return value  : the distance calculated 
*****************************************/

double calculateDistance(double bbox_width, double bbox_height) {
    // Known data points
    //double height1 = 500, d1 = 20.0;  // distance = 20cm
    //double height2 = 370, d2 = 27.0;  // distance = 30cm
    double height1 = 574, d1 = 20.0;  // distance = 20cm
    double height2 = 287, d2 = 27.0;  // distance = 30cm

    // Linear interpolation formula for height
    double distance = d1 + ((d2 - d1) / (height2 - height1)) * (bbox_height - height1);
return distance;
}
double calculateCorection(double distance) {
    // Known data points
    //double height1 = 500, d1 = 20.0;  // distance = 20cm
    //double height2 = 370, d2 = 27.0;  // distance = 30cm
    double d1 = 20, corectedd1 = 18.0;  // distance = 20cm
    double d2 = 27, correctedd2 = 28.0;  // distance = 30cm

    // Linear interpolation formula for height
    double newDistance = corectedd1 + ((correctedd2 - corectedd1) / (d2 - d1)) * (distance - d1);
return newDistance;
}
/**
 * @brief Performs the logistic sigmoid function on the input value
 * @param x Input
 * @return Output from the logistic function
 */
double sigmoid(double x) { return 1.0 / (1.0 + exp(-x)); }

/**
 * @brief Performs the softmax function on an array of values
 * @param val The array of values (modified in-place)
 */
void softmax(float val[NUM_CLASS]) {
  float max_num = -FLT_MAX;
  float sum = 0;
  int32_t i;
  for (i = 0; i < NUM_CLASS; i++) {
    max_num = std::max(max_num, val[i]);
  }

  for (i = 0; i < NUM_CLASS; i++) {
    val[i] = (float)exp(val[i] - max_num);
    sum += val[i];
  }

  for (i = 0; i < NUM_CLASS; i++) {
    val[i] = val[i] / sum;
  }
  return;
}

/**
 * @brief Returns the index to a specific bounding box "attribute", e.g., x co-ordinate, width, class, etc.
 * @param offs The offset to the start of the box's attribute, returned by offset()
 * @param channel The attribute number (0 = x, 1 = y, 2 = width, 3 = height, 4 = class)
 * @return Index into the inference results
 */
int32_t index(int32_t offs, int32_t channel) { return offs + channel * NUM_GRID_X * NUM_GRID_Y; }

/**
 * @brief Returns the offset to a given bounding box for a particular location in the output layer
 * @param b The bounding box number, indexed from 0
 * @param y The Y offset in the output layer
 * @param x The X offset in the output layer
 * @return Bounding box offset in the inference results
 */
int32_t offset(int32_t b, int32_t y, int32_t x) {
  return b * (NUM_CLASS + 5) * NUM_GRID_X * NUM_GRID_Y + y * NUM_GRID_X + x;
}

/**
 * @brief Performs post-processing on the output from the YOLOv2 model
 *
 * @param floatarr Inference results
 * @param detections Vector of detections (shared with the ImageThread)
 */
void performPostProcessing(float *floatarr, Detections detections) {
  std::lock_guard<std::mutex> lock(detections_mutex);

  // Correct region boxes variables
  float new_w, new_h;
  float correct_w = 1.;
  float correct_h = 1.;
  if ((float)(MODEL_IN_W / correct_w) < (float)(MODEL_IN_H / correct_h)) {
    new_w = (float)MODEL_IN_W;
    new_h = correct_h * MODEL_IN_W / correct_w;
  } else {
    new_w = correct_w * MODEL_IN_H / correct_h;
    new_h = MODEL_IN_H;
  }

  // Clear previous detections
  detections->clear();

  for (int b = 0; b < NUM_BB; b++) {
    for (int y = 0; y < NUM_GRID_Y; y++) {
      for (int x = 0; x < NUM_GRID_X; x++) {
        int offs = offset(b, y, x);
        float tx = floatarr[offs];
        float ty = floatarr[index(offs, 1)];
        float tw = floatarr[index(offs, 2)];
        float th = floatarr[index(offs, 3)];
        float tc = floatarr[index(offs, 4)];

        // Compute the bounding box
        float center_x = ((float)x + sigmoid(tx)) / (float)NUM_GRID_X;
        float center_y = ((float)y + sigmoid(ty)) / (float)NUM_GRID_Y;
        float box_w = (float)exp(tw) * anchors[2 * b + 0] / (float)NUM_GRID_X;
        float box_h = (float)exp(th) * anchors[2 * b + 1] / (float)NUM_GRID_Y;

//DANA
 // std::cout << "before ajustments"<< std::endl;
//  std::cout << "center_x : " <<center_x<< std::endl;
//  std::cout << "center_y :" << center_y<< std::endl;
//  std::cout << "box_w : " << box_w <<  std::endl;
//  std::cout << "box_h :" << box_h<< std::endl;

        /* Adjustment for VGA size */
        /* correct_region_boxes */
        center_x = (center_x - (MODEL_IN_W - new_w) / 2. / MODEL_IN_W) / ((float)new_w / MODEL_IN_W);
        center_y = (center_y - (MODEL_IN_H - new_h) / 2. / MODEL_IN_H) / ((float)new_h / MODEL_IN_H);
        box_w *= (float)(MODEL_IN_W / new_w);
        box_h *= (float)(MODEL_IN_H / new_h);

        center_x = round(center_x * DRPAI_IN_WIDTH);
        center_y = round(center_y * DRPAI_IN_HEIGHT);
        box_w = round(box_w * DRPAI_IN_WIDTH);
        box_h = round(box_h * DRPAI_IN_HEIGHT);

//DANA
//  std::cout << "after ajustments"<< std::endl;
//  std::cout << "center_x : " <<center_x<< std::endl;
//  std::cout << "center_y :" << center_y<< std::endl;
//  std::cout << "box_w : " << box_w <<  std::endl;
//  std::cout << "box_h :" << box_h<< std::endl;

        float objectness = sigmoid(tc);

        Box bb = {center_x, center_y, box_w, box_h};

        float classes[NUM_CLASS];
        for (int i = 0; i < NUM_CLASS; i++) {
          classes[i] = floatarr[index(offs, 5 + i)];
        }
        softmax(classes);

        float max_pred = 0;
        int pred_class = -1;

        for (int i = 0; i < NUM_CLASS; i++) {
          if (classes[i] > max_pred) {
            pred_class = i;
            max_pred = classes[i];
          }
        }

        float probability = max_pred * objectness;
        if (probability > TH_PROB) {
          detections->push_back({bb, pred_class, probability});
			// std::cout << "probability = " << probability <<  std::endl;
			 //std::cout << "pred_class = " << pred_class <<  std::endl;
			 //std::cout << "probability = " << probability<< std::endl;
        }
      }
    }
  }

  filter_boxes_nms(detections, TH_NMS);

  int number_of_boxes = 0;
  for (const Detection& detection : (*detections)) {
    if (detection.prob == 0) continue;
    spdlog::info(" Bounding Box        : (X, Y, W, H) = ({}, {}, {}, {})", static_cast<int>(detection.bbox.x),
                 static_cast<int>(detection.bbox.y), static_cast<int>(detection.bbox.w),
                 static_cast<int>(detection.bbox.h));
    spdlog::info(" Detected Class      : {} (Class {})", label_file_map[detection.c].c_str(), detection.c);
    spdlog::info(" Probability         : {} %", (std::round((detection.prob * 100) * 10) / 10));
    number_of_boxes++;
  }
  spdlog::info(" Bounding Box Count  : {}", number_of_boxes);
}

/**
 * @brief Draws bounding boxes on an image
 */
void drawBoundingBoxes(Image::SharedPtr image, Detections detections) {
  std::lock_guard<std::mutex> lock(detections_mutex);

    // static variables to persist across calls
    static bool object_was_detected = false;
    static int counts_detection = 0;

    bool object_found = false;
    double distance   = 0.0;
	std::string color;
	int height = 0;
	int width = 0;
if(!robot_is_moving.load())
{
  for (const Detection& detection : (*detections)) {
    if (detection.prob == 0 || detection.bbox.w>250) 
	{
		//object_was_detected = false;
	}
else
{
	int minWidthBoxDetectionZone = 700;
	int maxWidthBoxDetectionZone = 1250;
	int minHeightBoxDetectionZone = 250;
	int maxHeightBoxDetectionZone = 630;
double centerX = detection.bbox.x + (detection.bbox.w / 2.0);
double centerY = detection.bbox.y + (detection.bbox.h / 2.0);
if(((static_cast<int>(detection.bbox.x))>minWidthBoxDetectionZone)&&((static_cast<int>(detection.bbox.x))<maxWidthBoxDetectionZone))//&&((static_cast<int>(detection.bbox.y))>minHeightBoxDetectionZone)&&((static_cast<int>(detection.bbox.y))<maxHeightBoxDetectionZone))
{
    std::stringstream stream;
    stream << std::fixed << std::setprecision(2) << detection.prob;

        // Calculate distance
    distance = calculateDistance(centerX, centerY);
		height = static_cast<int>(centerY);
		width = static_cast<int>(centerX);
	//std::cout << "distance is as follows = "<<distance<<std::endl;
        // Define the bounding box as a cv::Rect
        cv::Rect bbox(static_cast<int>(detection.bbox.x), static_cast<int>(detection.bbox.y),
                      static_cast<int>(detection.bbox.w), static_cast<int>(detection.bbox.h));

// Get the dominant color inside the bounding box
	color = image->getDominantColor(static_cast<int>(detection.bbox.x), static_cast<int>(detection.bbox.y),
                         static_cast<int>(detection.bbox.w), static_cast<int>(detection.bbox.h));
	std::ostringstream distance_stream;
	distance_stream << std::fixed << std::setprecision(2) << distance; 
    // Append distance and colour to the label 
    std::string text = label_file_map[detection.c] + " " + stream.str() +
                       " D: " + distance_stream.str() + "cm" + " "+color;
                       
    image->drawRectangle(static_cast<int>(detection.bbox.x), static_cast<int>(detection.bbox.y),
                         static_cast<int>(detection.bbox.w), static_cast<int>(detection.bbox.h), text,
                         box_color[detection.c]);
		object_found = true;
		std::cout << "distance  = "<< distance<<"  hightP = "<<height<<"  widthP = "<<width<<"  hight = "<<detection.bbox.w<<"  width = "<<detection.bbox.h<<std::endl;
}
else
{
//object detected are not inside the area
}
}
}
//robot_is_moving.store(true);
// Now handle the stable detection logic
    if ((object_found)&&(!robot_is_moving.load())) {
        if (!object_was_detected) {
            // This is the first time we see it
            object_was_detected = true;
        } else {
		counts_detection++;	
				std::cout << "counts_detection = " << counts_detection<<std::endl;
            if (counts_detection > 10) {
                // object is stable for 1.5s
                // if robot not moving, call moveToTarget(distance, 0)
				counts_detection = 0;
                if (!robot_is_moving.load()) {
					if((distance>=15)&&(distance <35))
					{
					robot_is_moving.store(true);
					//double correctedDistance = calculateCorection(distance);
					if(color == "Blue")
					{	
							std::cout << "MoveToTarget left Blue and distance  = "<< distance<<"  hightP = "<<height<<"  widthP = "<<width<<std::endl;
							sMoveToTarget->enqueueTarget(distance, 2.0,1);
					}
					else if(color == "Red")
					{	
							std::cout << "MoveToTarget left Red and distance  = "<< distance<<"  hightP = "<<height<<"  widthP = "<<width<<std::endl;
							sMoveToTarget->enqueueTarget(distance, 2.0,2);
					}
					else if(color == "Yellow")
					{	
							std::cout << "MoveToTarget left Yellow and distance  = "<< distance<<"  hightP = "<<height<<"  widthP = "<<width<<std::endl;
							sMoveToTarget->enqueueTarget(distance, 2.0,4);
					}
					else if(color == "Green")
					{

						std::cout << "MoveToTarget right Green and distance  = "<< distance<<"  hightP = "<<height<<"  widthP = "<<width<<std::endl;
						sMoveToTarget->enqueueTarget(distance, 2.0,3);
					}
					}
                }
            }
        }
    } else {
        // No object found this frame
        object_was_detected = false;
    }
}
}

/**
 * @brief Draws statistics and metrics on the image
 */
void drawStatistics(Image::SharedPtr image, Statistics::SharedPtr stats) {
  std::stringstream stream;

  stream.str("");
  stream << "Pre-Proc + Inference (DRP-AI): " << std::setw(3) << std::fixed << std::setprecision(1)
         << std::round(stats->inference_time_ms * 10) / 10 << "msec";
  image->drawText(stream.str(), TextAlignment::kRightAligned, TEXT_WIDTH_OFFSET, LINE_HEIGHT_OFFSET + (LINE_HEIGHT * 1),
                  CHAR_SCALE_LARGE, 0xFFF000u);

  stream.str("");
  stream << "Post-Proc (CPU): " << std::setw(3) << std::fixed << std::setprecision(1)
         << std::round(stats->post_processing_time_ms * 10) / 10 << "msec";
  image->drawText(stream.str(), TextAlignment::kRightAligned, TEXT_WIDTH_OFFSET, LINE_HEIGHT_OFFSET + (LINE_HEIGHT * 2),
                  CHAR_SCALE_LARGE, 0xFFF000u);

  stream.str("");
  stream << "AI/Camera Frame Rate: " << std::setw(3) << (uint32_t)stats->average_inference_frame_rate << "/"
         << (uint32_t)stats->average_capture_frame_rate << "fps";
  image->drawText(stream.str(), TextAlignment::kRightAligned, TEXT_WIDTH_OFFSET, LINE_HEIGHT_OFFSET + (LINE_HEIGHT * 3),
                  CHAR_SCALE_LARGE, 0xFFF000u);
}

/**
 * @brief Implements the body of the main inference thread
 * @param drp_ai_accelerator DRP-AI accelerator object
 * @param drp_ai_buffer DMA buffer for the DRP-AI accelerator
 * @param detections Vector of Detection objects
 * @param stats Shared statistics
 */
void InferenceThread(DrpAiAccelerator::SharedPtr drp_ai_accelerator, DmaBuffer::SharedPtr drp_ai_buffer,
                     Detections detections, Statistics::SharedPtr stats) {
  // Output buffer
  std::vector<float> output_buffer(INF_OUT_SIZE, 0.);

  // Contains the last 30 frame interval times; used to compute the average "AI frame rate"
  std::deque<double> last_n_frame_intervals(30, 1000.);

  std::cout << "Inference Thread Starting" << std::endl;
  std::cout << "Inference Loop Starting" << std::endl;

  // Frame interval timer; we start and stop it to avoid crazy results on the first pass
  Timer frame_interval_timer;
  frame_interval_timer.start();
  frame_interval_timer.stop();

  int inference_count{0};
  while (1) {
    inference_count++;
    spdlog::info("[START] Start DRP-AI Inference...");
    spdlog::info("Inference ----------- No. {}", inference_count);

    while (1) {
      // Check to see if the application has been terminated
      if (terminationRequested()) {
        goto ai_inf_end;
      }

      // Wait until the capture thread has received an image
      if (inference_start.load()) {
        break;
      }

      std::this_thread::sleep_for(kThreadSleepTime);
    }

    Timer drp_ai_timer;
    drp_ai_timer.start();

    if (!drp_ai_accelerator->runInference((uintptr_t)drp_ai_buffer->physicalAddress())) {
      std::cerr << "[ERROR] Failed to run inference" << std::endl;
      goto err;
    }

    fd_set rfds;
    FD_ZERO(&rfds);
    FD_SET(drp_ai_accelerator->getFd(), &rfds);

    timespec timeout;
    timeout.tv_sec = DRPAI_TIMEOUT;
    timeout.tv_nsec = 0;

    int ret = pselect(drp_ai_accelerator->getFd() + 1, &rfds, NULL, NULL, &timeout, NULL);

    if (0 == ret) {
      std::cerr << "[ERROR] DRP-AI Inference pselect() Timeout: errno=" << errno << std::endl;
      goto err;
    } else if (0 > ret) {
      std::cerr << "[ERROR] DRP-AI Inference pselect() Error: errno=" << errno << std::endl;

      drpai_status_t drpai_status0;
      ret = ioctl(drp_ai_accelerator->getFd(), DRPAI_GET_STATUS, &drpai_status0);
      if (-1 == ret) {
        std::cerr << "[ERROR] Failed to run DRPAI_GET_STATUS : errno=" << errno << std::endl;
      }
      goto err;
    }

    drp_ai_timer.stop();

    // Record the inference time
    stats->inference_time_ms = drp_ai_timer.elapsed().count() * 1000.;

    drpai_status_t drpai_status0;
    if (0 == ioctl(drp_ai_accelerator->getFd(), DRPAI_GET_STATUS, &drpai_status0)) {
      // Get the inference results
      if (!drp_ai_accelerator->getResult(output_buffer.data(), output_buffer.size() * sizeof(float))) {
        std::cerr << "[ERROR] Failed to get result from memory" << std::endl;
        goto err;
      }

      Timer post_processing_timer;
      post_processing_timer.start();

      // Perform the post-processing on the CPU
      performPostProcessing(output_buffer.data(), detections);

      post_processing_timer.stop();
      stats->post_processing_time_ms = post_processing_timer.elapsed().count() * 1000.;

      frame_interval_timer.stop();
      // Record the latest frame interval
      const double frame_interval = frame_interval_timer.elapsed().count() * 1000.;
      frame_interval_timer.start();

      // Update the list of frame intervals
      last_n_frame_intervals.pop_front();
      last_n_frame_intervals.push_back(frame_interval);

      spdlog::info("Pre-Proc + Inference (DRP-AI): {} [ms]", std::round(stats->inference_time_ms * 10) / 10);
      spdlog::info("Post-Proc (CPU): {} [ms]", std::round(stats->post_processing_time_ms * 10) / 10);

      // Compute the average "AI frame rate"
      const double frame_interval_average =
          std::accumulate(last_n_frame_intervals.begin(), last_n_frame_intervals.end(), 0) /
          last_n_frame_intervals.size();
      stats->average_inference_frame_rate = 1.0 / frame_interval_average * 1000.0 + 0.5;

      spdlog::info("AI Frame Rate {} [fps]", (int32_t)stats->average_inference_frame_rate);
    } else {
      std::cerr << "[ERROR] Failed to get DRPAI status" << std::endl;
      goto err;
    }

    inference_start.store(0);
  }

err:
  sem_trywait(&terminate_req_sem);

ai_inf_end:
  std::cout << "AI Inference Thread Terminated" << std::endl;
}

/**
 * @brief Implments the body of the capture thread
 * @param capture Camera device
 * @param wayland_buffer
 * @param drp_ai_buffer
 */
void CaptureThread(Camera::SharedPtr camera, DmaBuffer::SharedPtr wayland_buffer, DmaBuffer::SharedPtr drp_ai_buffer,
                   Image::SharedPtr image, Statistics::SharedPtr stats) {
  // Number of frames to discard before starting inference
  uint8_t discard_frame_count{8};

  // Contains the last 30 frame interval times; used to compute the average "camera frame rate"
  std::deque<double> last_n_frame_intervals(30, 1000.);
  std::cout << "Capture Thread Starting" << std::endl;

  uint8_t *img_buffer0 = static_cast<uint8_t *>(drp_ai_buffer->mem());

  // Frame interval timer; we start and stop it to avoid crazy results on the first pass
  Timer frame_interval_timer;
  frame_interval_timer.start();
  frame_interval_timer.stop();

  while (1) {
    // Check to see if the application has been terminated
    if (terminationRequested()) {
		std::cout << "Capture Thread Terminated" << std::endl;
      goto capture_end;
    }
    frame_interval_timer.stop();
    // Record the latest frame interval
    const double frame_interval = frame_interval_timer.elapsed().count() * 1000.;
    frame_interval_timer.start();

    // Update the list of frame intervals
    last_n_frame_intervals.pop_front();
    last_n_frame_intervals.push_back(frame_interval);

    const double frame_interval_average =
        std::accumulate(last_n_frame_intervals.begin(), last_n_frame_intervals.end(), 0) /
        last_n_frame_intervals.size();

    stats->average_capture_frame_rate = 1.0 / frame_interval_average * 1000.0 + 0.5;

    if (!camera->captureImage()) {
      std::cerr << "[ERROR] Failed to capture image from camera" << std::endl;
	  std::cout << "[ERROR] Failed to capture image from camera" << std::endl;
      goto err;
    }

    /* Do not process until the camera stabilizes, because the image is unreliable until the camera stabilizes. */
    if (discard_frame_count > 0) {
      discard_frame_count--;
    } else 
	{
      uint8_t *capture_buffer_data = camera->getCaptureBufferData();
		
      if (!inference_start.load()) {
        // Copy the captured image to the DRP-AI buffer
        memcpy(img_buffer0, capture_buffer_data, camera->getCaptureBufferSize());
        // Flush the DRP-AI DMA buffer
        if (!drp_ai_buffer->flush()) {
		std::cout << "failed to flush drp_ai_buffer" << std::endl;
          goto err;
        }

        // Notify the inference thread
        inference_start.store(1);
      }

      if (!img_obj_ready.load()) {
        // Copy the camera image to the Wayland buffer, ready for annotation and display
        image->copyCameraDataToWaylandBuffer(capture_buffer_data, camera->getCaptureBufferSize());

        // Flush the Wayland DMA buffer
        if (!wayland_buffer->flush()) {
		  std::cout << "failed to flush wayland buffer " << std::endl;
          goto err;
        }

        // Notify the image thread
        img_obj_ready.store(1);
      }
    }

    // Re-queue the capture buffer
    if (!camera->queueCaptureBuffer()) {
      std::cerr << "[ERROR] Failed to enqueue capture buffer" << std::endl;
	  std::cout << "[ERROR] Failed to enqueue capture buffer" << std::endl;
      goto err;
    }
  }

err:
  sem_trywait(&terminate_req_sem);

capture_end:
  inference_start.store(1);

  std::cout << "Capture Thread Terminated" << std::endl;
}

/**
 * @brief Performs image processing on the captured image
 */
/**
 * @brief Performs image processing on the captured image
 */
void ImageThread(Image::SharedPtr image, Detections detections, Statistics::SharedPtr stats) {
    // We allocate one RGBA buffer large enough to hold the entire displayed image
    static std::vector<uint8_t> rgba_buffer(
        IMAGE_OUTPUT_WIDTH * IMAGE_OUTPUT_HEIGHT * 4);
  std::cout << "Image Thread Starting" << std::endl;
    uint8_t * img_buffer0;

    img_buffer0 = (unsigned char*) (malloc(IMAGE_OUTPUT_HEIGHT*IMAGE_OUTPUT_WIDTH*BGRA_CHANNEL));
  int wayland_start = wayland.init(0, IMAGE_OUTPUT_WIDTH, IMAGE_OUTPUT_HEIGHT, BGRA_CHANNEL,true);
  while (1) {

        // 1) Check if termination was requested
    if (terminationRequested()) {
	std::cout << "Image Thread Terminated" << std::endl;
      goto image_end;
    }
         // 2) Check if a new image is ready
        if (img_obj_ready.load() == 1)
        {
            // 2a) Convert from YUY2 -> BGR
            //     (Your code typically calls `image->convertToBGR();` which modifies the active buffer in-place)
            image->convertToBGR(); 

            // 2b) Optionally resize the BGR image (if you want a specific output size)
            image->resizeImage(DRPAI_OUT_WIDTH);

            // 2c) Draw bounding boxes + stats (both operate on the BGR buffer)
            drawBoundingBoxes(image, detections);
            drawStatistics(image, stats);

            // 2d) Convert from BGR -> RGBA
            //     The active buffer in `Image` is BGR and has 3 channels.
            uint8_t* bgr_data = image->getWaylandBufferData(image->getActiveBufferId());
            size_t pixelCount = IMAGE_OUTPUT_WIDTH * IMAGE_OUTPUT_HEIGHT;
            for (size_t i = 0; i < pixelCount; i++)
            {
                uint8_t B = bgr_data[i*3 + 0];
                uint8_t G = bgr_data[i*3 + 1];
                uint8_t R = bgr_data[i*3 + 2];
                rgba_buffer[i*4 + 0] = B;
                rgba_buffer[i*4 + 1] = G;
                rgba_buffer[i*4 + 2] = R;
                rgba_buffer[i*4 + 3] = 255;  // Full alpha
            }
            // Create OpenCV Mat headers around our existing data
            //cv::Mat bgrMat(IMAGE_OUTPUT_HEIGHT, IMAGE_OUTPUT_WIDTH, CV_8UC3, bgr_data);
            //cv::Mat bgraMat(IMAGE_OUTPUT_HEIGHT, IMAGE_OUTPUT_WIDTH, CV_8UC4,cv::Scalar(0, 0, 0,0) );

            // Convert BGR to BGRA (adds an alpha channel)
            //cv::cvtColor(bgrMat, bgraMat, cv::COLOR_BGR2BGRA);

			//memcpy(img_buffer0, bgraMat.data, IMAGE_OUTPUT_HEIGHT * IMAGE_OUTPUT_WIDTH * BGRA_CHANNEL);
            // 2e) Commit to Wayland

            wayland.commit(rgba_buffer.data(), nullptr);
			//wayland.commit(img_buffer0, nullptr);

            // 2f) Reset the flag so we wait for the next image
            img_obj_ready.store(0);
        }

    std::this_thread::sleep_for(kThreadSleepTime);
  }

image_end:
  img_obj_ready.store(0);
  wayland.exit();
  std::cout << "Img Thread Terminated" << std::endl;
}

/**
 * @brief Monitors the keyboard for key presses, and halts the application if <enter> is pressed
 */
void KeyboardThread() {
  std::cout << "Key Hit Thread Starting" << std::endl;

  std::cout << "************************************************" << std::endl;
  std::cout << "* Press ENTER key to quit. *" << std::endl;
  std::cout << "************************************************" << std::endl;

  /*Set Standard Input to Non Blocking*/
  if (-1 == fcntl(0, F_SETFL, O_NONBLOCK)) {
    std::cerr << "[ERROR] Failed to run fctnl()" << std::endl;
    goto err;
  }

  while (1) {
    if (terminationRequested()) {
      goto key_hit_end;
    }

    int c = getchar();
    if (EOF != c) {
      std::cout << "Key Detected" << std::endl;
      goto err;
    } else {
      std::this_thread::sleep_for(kThreadSleepTime);
    }
  }

err:
  sem_trywait(&terminate_req_sem);

key_hit_end:
  std::cout << "Key Hit Thread Terminated" << std::endl;
}

/**
 * @brief Implements the main busy loop for the application
 *
 * @return
 */
bool RunMainLoop() {
  bool okay{true};
  std::cout << "Main Loop Starts" << std::endl;
int x =0;
  while (1) {
 //   if (terminationRequested()) {
 //     goto main_proc_end;
 //   }
//	x++;
//	if(x==1000)
//{
//	openGripRobot();
//}
//	if(x==5000)
//{
//	closeGripRobot();
//	x=0;
//}
    std::this_thread::sleep_for(kThreadSleepTime);
  }

main_proc_end:
  std::cout << "Main Process Terminated" << std::endl;
  return okay;
}

static constexpr std::array<Camera::CameraSource, 6> camera_src_map {
  Camera::CameraSource::CRU0,
  Camera::CameraSource::CRU1,
  Camera::CameraSource::UVC0,
  Camera::CameraSource::UVC1
};

/**
 * @brief Entry point
 *
 * @param argc
 * @param argv
 * @return int32_t
 */
int32_t main(int32_t argc, char *argv[]) {
  argparse::ArgumentParser program_options("app-yolov2-demo");
  program_options.add_argument("-c","--config")
    .help("Full path to JSON config file to configure the Camera Stream")
    .default_value(std::string{}); // empty string

  program_options.add_argument("--drp_clk_div")
    .help("DRP Clock Divider [Unsigned Int]")
    .default_value(kDefaultDrpClockDivider);

  program_options.add_argument("--ai_mac_clk_div")
    .help("AI MAC Clock Divider [Unsigned Int]")
    .default_value(kDefaultAiMacClockDivider);

  try {
    program_options.parse_args(argc, argv);
  }
  catch (const std::exception& err) {
    std::cerr << err.what() << std::endl;
    std::cerr << program_options;
    return -EXIT_FAILURE;
  }

  namespace fs = std::filesystem;
  using json = nlohmann::json;

  Camera::CameraSource selected_camera_src = Camera::CameraSource::UVC0;
	std::string config_path = program_options.get<std::string>("--config");
	if (!config_path.empty() && std::filesystem::exists(config_path)) {
		// parse the JSON
		std::ifstream stream_config_file{config_path};
		json stream_config = json::parse(stream_config_file);
		selected_camera_src = camera_src_map.at(stream_config["selected_sensor"].get<uint8_t>());
	} else {
		// No config argument or file missing
		// Keep your fallback: e.g. selected_camera_src = Camera::CameraSource::UVC0
		std::cout << "[INFO] No JSON config provided (or file not found). Defaulting to USB camera.\n";
		selected_camera_src = Camera::CameraSource::UVC0;
	}

  // DRP-AI clock dividers
  uint32_t drp_clock_divider{program_options.get<uint32_t>("--drp_clk_div")};
  uint32_t ai_mac_clock_divider{program_options.get<uint32_t>("--ai_mac_clk_div")};

  auto now = std::chrono::system_clock::now();
  auto logger = spdlog::basic_logger_mt("logger", fmt::format("logs/{:%Y-%m-%d_%H-%M-%S}_app_yolov2_cam.log", now));
  spdlog::set_default_logger(logger);

  std::cout << "RZ/V2H DRP-AI Sample Application" << std::endl;
  std::cout << "Built : " << __DATE__ << " " << __TIME__ << std::endl;
  std::cout << "Model : Darknet YOLOv2 | " << drpai_prefix0 << std::endl;
  std::cout << "Input : " << INPUT_CAM_NAME << std::endl;
  std::cout << "Cam   : Camera" << static_cast<int>(selected_camera_src) << std::endl;

  spdlog::info("************************************************");
  spdlog::info("  RZ/V2H DRP-AI Sample Application");
  spdlog::info("  Model : Darknet YOLOv2 | {}", drpai_prefix0.c_str());
  spdlog::info("  Input : {}", INPUT_CAM_NAME);
  spdlog::info("************************************************");

  // Input and output dimensions
  ImageDimensions input_dimensions{CAM_IMAGE_WIDTH, CAM_IMAGE_HEIGHT, CAM_IMAGE_CHANNEL_YUY2};
  ImageDimensions output_dimensions{IMAGE_OUTPUT_WIDTH, IMAGE_OUTPUT_HEIGHT, IMAGE_CHANNEL_BGR};

  std::cout << "input_dimensions : " << CAM_IMAGE_WIDTH<<CAM_IMAGE_HEIGHT<<CAM_IMAGE_CHANNEL_YUY2<< std::endl;
  std::cout << "output_dimensions" << IMAGE_OUTPUT_WIDTH<<IMAGE_OUTPUT_HEIGHT<<IMAGE_CHANNEL_BGR<< std::endl;

  // DRP-AI driver
  DrpAiAccelerator::SharedPtr drp_ai_accelerator{nullptr};

  try {
    drp_ai_accelerator = std::make_shared<DrpAiAccelerator>();
  } catch (std::runtime_error& e) {
    std::cerr << e.what() << std::endl;
    return -EXIT_FAILURE;
  }

  if (!drp_ai_accelerator->loadObjectFiles(drpai_prefix0)) {
    std::cerr << "[ERROR] Failed to load object files" << std::endl;
    return -EXIT_FAILURE;
  }

  if (!drp_ai_accelerator->setClockDividers(drp_clock_divider, ai_mac_clock_divider)) {
    return -EXIT_FAILURE;;
  }

  // Camera driver
  auto camera = std::make_shared<Camera>(selected_camera_src, 
                                         input_dimensions.width, 
                                         input_dimensions.height);
  if (!camera->startCamera()) {
    std::cerr << "Failed to initialise the camera" << std::endl;
    return -EXIT_FAILURE;
  }

  static constexpr uint32_t kNumberOfOutputBuffers{4U};
  auto output_buffer = std::make_shared<DmaBuffer>(output_dimensions.sizeBytes(), kNumberOfOutputBuffers);
  auto drp_ai_buffer = std::make_shared<DmaBuffer>(input_dimensions.sizeBytes());
  auto image = std::make_shared<Image>(input_dimensions, output_dimensions, output_buffer);
  auto detections = std::make_shared<std::vector<Detection>>();
  auto stats = std::make_shared<Statistics>();
	
  if (0 != sem_init(&terminate_req_sem, 0, kExpectedSemaphoreValue)) {
    std::cerr << "[ERROR] Failed to Initialize Termination Request Semaphore" << std::endl;
    return -EXIT_FAILURE;
  }

    // 1. Export the two GPIO pins 461 and 462
    std::system("echo 461 > /sys/class/gpio/export");
    std::system("echo 462 > /sys/class/gpio/export");

    // 2. Set their direction to "in" (input)

    std::system("echo in > /sys/class/gpio/P5_5/direction");
    std::system("echo in > /sys/class/gpio/P5_6/direction");


  std::thread keyboard_thread(KeyboardThread);
  std::thread inference_thread(InferenceThread, drp_ai_accelerator, drp_ai_buffer, detections, stats);
  std::thread capture_thread(CaptureThread, camera, output_buffer, drp_ai_buffer, image, stats);
  std::thread image_thread(ImageThread, image, detections, stats);
    // Start the GPIO thread
  std::thread gpio_thread(GpioButtonThread);

    std::cout << "Starting Cube Detection Application" << std::endl;

	 //Init I2C 
	sRobotControl;
	sMoveToTarget;
    //std::this_thread::sleep_for(std::chrono::seconds(5));
	sRobotControl->startWorker();
	sMoveToTarget->startWorker();
	bool checkInitUART = sMoveToTarget->initUART();
	if(checkInitUART)
	{
		std::cout << "Init UART succesfull" << std::endl;
	}
	else
	{
		std::cout << "Init UART failed" << std::endl;
	}


  int32_t exit_code{EXIT_SUCCESS};
  if (!RunMainLoop()) {
    std::cerr << "[ERROR] Error during Main Process" << std::endl;
    exit_code = -EXIT_FAILURE;
  }

  image_thread.join();
  capture_thread.join();
  inference_thread.join();
  keyboard_thread.join();
// if exited from main loop, close worker thread
	sRobotControl->stopWorker();
	//close gpio button thread
	gShouldTerminate.store(true);
	gpio_thread.join();

  sem_destroy(&terminate_req_sem);

  std::cout << "Application End" << std::endl;
  return exit_code;
}
