/*
 * Original Code (C) Copyright Edgecortix, Inc. 2022
 * Modified Code (C) Copyright Renesas Electronics Corporation 2023
 * ROS2 Node Adaptation 2024
 *
 *  *1 DRP-AI TVM is powered by EdgeCortix MERA(TM) Compiler Framework.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/***********************************************************************************************************************
 * File Name    : hand_gesture_recognition_node.cpp
 * Version      : 1.1.0
 * Description  : ROS2 Node for Hand Gesture Recognition using YOLOv3 on RZ/V2N DRP-AI.
 *                Camera opened directly via V4L2 (no GStreamer).
 *                Detection image published using the BGRA->BGR cv_bridge pattern
 *                from darknet_drp_ros.
 *
 *                Publishes:
 *                  - /hand_gesture/detection  (std_msgs/String)   : detected gesture label
 *                  - /hand_gesture/image_raw  (sensor_msgs/Image) : annotated BGR image
 *                  - /hand_gesture/timing     (std_msgs/String)   : JSON timing diagnostics
 ***********************************************************************************************************************/

#include "hand_gesture_recognition/define.h"
#include "hand_gesture_recognition/box.h"
#include "MeraDrpRuntimeWrapper.h"

#include <linux/drpai.h>
#include <builtin_fp16.h>
#include <opencv2/opencv.hpp>

/* ROS2 headers */
#include <rclcpp/rclcpp.hpp>
#include <std_msgs/msg/string.hpp>
#include <sensor_msgs/msg/image.hpp>
#include <sensor_msgs/msg/compressed_image.hpp>
#include <cv_bridge/cv_bridge.h>

#include <sstream>
#include <iomanip>
#include <chrono>
#include <limits.h>   /* PATH_MAX for realpath() */
#include <mutex>
#include <thread>
#include <atomic>

using namespace std;
using namespace cv;

/*========================================================
 *  Helper: FP16 -> FP32 conversion
 *========================================================*/
static float float16_to_float32(uint16_t a)
{
    return __extendXfYf2__<uint16_t, uint16_t, 10, float, uint32_t, 23>(a);
}

/*========================================================
 *  Helper: load label file
 *========================================================*/
static vector<string> load_label_file(const string & label_file_name)
{
    vector<string> list, empty;
    ifstream infile(label_file_name);
    if (!infile.is_open()) return list;
    string line;
    while (getline(infile, line)) {
        list.push_back(line);
        if (infile.fail()) return empty;
    }
    return list;
}

/*========================================================
 *  YOLOv3 post-processing helpers
 *========================================================*/
static double sigmoid(double x) { return 1.0 / (1.0 + exp(-x)); }

static int32_t yolo_index(uint8_t n, int32_t offs, int32_t channel)
{
    uint8_t num_grid = num_grids[n];
    return offs + channel * num_grid * num_grid;
}

static int32_t yolo_offset(uint8_t n, int32_t b, int32_t y, int32_t x)
{
    uint8_t num = num_grids[n];
    uint32_t prev_layer_num = 0;
    for (int32_t i = 0; i < n; i++)
        prev_layer_num += NUM_BB * (NUM_CLASS + 5) * num_grids[i] * num_grids[i];
    return prev_layer_num + b * (NUM_CLASS + 5) * num * num + y * num + x;
}

/*========================================================
 *  ROS2 Node class
 *========================================================*/
class HandGestureRecognitionNode : public rclcpp::Node
{
public:
    explicit HandGestureRecognitionNode(const rclcpp::NodeOptions & options = rclcpp::NodeOptions())
    : Node("hand_gesture_recognition", options)
    {
        /* ---- Declare & get parameters ---- */
        this->declare_parameter<int>("drpai_freq",              DRPAI_FREQ);
        this->declare_parameter<std::string>("model_dir",       model_dir);
        this->declare_parameter<std::string>("label_list",      label_list);
        this->declare_parameter<std::string>("camera_device",   "");  /* empty = auto-detect */
        this->declare_parameter<std::string>("image_topic",     "/hand_gesture/image_raw");
        this->declare_parameter<std::string>("detection_topic", "/hand_gesture/detection");
        this->declare_parameter<std::string>("timing_topic",    "/hand_gesture/timing");
        this->declare_parameter<int>("jpeg_quality",     80);   /* 1-100 */
        this->declare_parameter<int>("image_publish_every", 1); /* publish every N inference frames */

        drpai_freq_          = this->get_parameter("drpai_freq").as_int();
        model_dir_param_     = this->get_parameter("model_dir").as_string();
        label_list_param_    = this->get_parameter("label_list").as_string();
        camera_device_       = this->get_parameter("camera_device").as_string();
        jpeg_quality_        = this->get_parameter("jpeg_quality").as_int();
        image_publish_every_ = this->get_parameter("image_publish_every").as_int();

        const auto img_topic    = this->get_parameter("image_topic").as_string();
        const auto det_topic    = this->get_parameter("detection_topic").as_string();
        const auto timing_topic = this->get_parameter("timing_topic").as_string();

        /* ---- Publishers ---- */
        pub_detection_ = this->create_publisher<std_msgs::msg::String>(det_topic, 10);
        pub_image_     = this->create_publisher<sensor_msgs::msg::CompressedImage>(
                             img_topic + "/compressed", rclcpp::SensorDataQoS());
        pub_timing_    = this->create_publisher<std_msgs::msg::String>(timing_topic, 10);

        RCLCPP_INFO(this->get_logger(), "Hand Gesture Recognition node starting...");
        RCLCPP_INFO(this->get_logger(), "  Model dir    : %s", model_dir_param_.c_str());
        RCLCPP_INFO(this->get_logger(), "  Label list   : %s", label_list_param_.c_str());
        RCLCPP_INFO(this->get_logger(), "  DRP-AI freq  : %d", drpai_freq_);
        RCLCPP_INFO(this->get_logger(), "  Image topic  : %s", img_topic.c_str());
        RCLCPP_INFO(this->get_logger(), "  Gesture topic: %s", det_topic.c_str());

        /* ---- Initialise DRP-AI ---- */
        if (!init_drpai_runtime()) {
            throw std::runtime_error("[ERROR] Failed to initialise DRP-AI runtime.");
        }

        /* ---- Auto-detect USB camera if no device was specified ---- */
        if (camera_device_.empty()) {
            camera_device_ = detect_usb_camera();
            if (camera_device_.empty()) {
                throw std::runtime_error("[ERROR] No USB camera found. "
                    "Set the 'camera_device' parameter explicitly (e.g. /dev/video0).");
            }
            RCLCPP_INFO(this->get_logger(), "Auto-detected USB camera: %s", camera_device_.c_str());
        } else {
            RCLCPP_INFO(this->get_logger(), "Using specified camera device: %s", camera_device_.c_str());
        }

        /* ---- Open camera via V4L2 ---- */
        cap_.open(camera_device_, cv::CAP_V4L2);
        if (!cap_.isOpened()) {
            throw std::runtime_error("[ERROR] Cannot open camera: " + camera_device_);
        }
        cap_.set(cv::CAP_PROP_FRAME_WIDTH,  IMAGE_WIDTH);
        cap_.set(cv::CAP_PROP_FRAME_HEIGHT, IMAGE_HEIGHT);
        cap_.set(cv::CAP_PROP_FPS, 30);
        /* Keep the V4L2 internal buffer small so we always get the freshest frame */
        cap_.set(cv::CAP_PROP_BUFFERSIZE, 2);
        double actual_fps = cap_.get(cv::CAP_PROP_FPS);
        RCLCPP_INFO(this->get_logger(), "Camera opened: %s (%dx%d @ %.0f fps)",
                    camera_device_.c_str(), IMAGE_WIDTH, IMAGE_HEIGHT, actual_fps);

        /* ---- Camera capture thread ----------------------------------------
         * Runs independently of inference. Continuously grabs frames and
         * stores the latest one in latest_frame_. The inference timer picks
         * it up without ever blocking on the camera.
         * ------------------------------------------------------------------ */
        capture_running_ = true;
        capture_thread_  = std::thread(&HandGestureRecognitionNode::capture_loop, this);

        /* ---- Main inference timer (100 ms = 10 fps target) ---- */
        timer_ = this->create_wall_timer(
            std::chrono::milliseconds(100),
            std::bind(&HandGestureRecognitionNode::inference_callback, this));
    }

    ~HandGestureRecognitionNode()
    {
        /* Stop capture thread first, then release camera */
        capture_running_ = false;
        if (capture_thread_.joinable()) capture_thread_.join();
        cap_.release();
        if (drpai_fd_ >= 0) close(drpai_fd_);
    }

private:
    /* -----------------------------------------------------------------------
     *  Auto-detect the first USB video device via /sys/class/video4linux
     *
     *  Walks /sys/class/video4linux/videoN/name and returns the first
     *  /dev/videoN whose driver name contains "usb" (case-insensitive).
     *  Returns an empty string if nothing is found.
     * --------------------------------------------------------------------- */
    std::string detect_usb_camera()
    {
        /* Walk /sys/class/video4linux/videoN and resolve the symlink to find
         * whether the device is on a USB bus. This works for any USB camera
         * (UVC, Logitech, etc.) regardless of its reported name, because the
         * resolved sysfs path always contains "/usb" for USB-attached devices.
         *
         * Example resolved path for a Logitech C270:
         *   /sys/devices/platform/.../usb1/1-1/1-1:1.0/video4linux/video0
         */
        for (int i = 0; i < 16; i++) {
            std::string dev     = "/dev/video" + std::to_string(i);
            std::string syslink = "/sys/class/video4linux/video" + std::to_string(i);

            /* Resolve the symlink to get the real sysfs device path */
            char resolved[PATH_MAX] = {};
            if (realpath(syslink.c_str(), resolved) == nullptr) continue;

            std::string real_path(resolved);

            /* USB devices always have "/usb" somewhere in their sysfs path */
            if (real_path.find("/usb") != std::string::npos) {
                /* Read friendly name for the log message */
                std::string name = "unknown";
                std::ifstream f(syslink + "/name");
                if (f.is_open()) std::getline(f, name);

                RCLCPP_INFO(this->get_logger(),
                            "Auto-detected USB camera: %s (%s)", dev.c_str(), name.c_str());
                return dev;
            }
        }
        return "";
    }

    /* -----------------------------------------------------------------------
     *  DRP-AI / runtime initialisation
     * --------------------------------------------------------------------- */
    bool init_drpai_runtime()
    {
        /* Disable OpenCV Accelerator for single-thread safety */
        unsigned long OCA_list[16] = {};

        OCA_Activate(&OCA_list[0]);

        errno = 0;
        drpai_fd_ = open("/dev/drpai0", O_RDWR);
        if (drpai_fd_ < 0) {
            RCLCPP_ERROR(this->get_logger(),
                         "[ERROR] Failed to open DRP-AI driver: errno=%d", errno);
            return false;
        }

        drpai_data_t drpai_data;
        if (ioctl(drpai_fd_, DRPAI_GET_DRPAI_AREA, &drpai_data) < 0) {
            RCLCPP_ERROR(this->get_logger(),
                         "[ERROR] DRPAI_GET_DRPAI_AREA failed: errno=%d", errno);
            return false;
        }

        label_file_map_ = load_label_file(label_list_param_);
        if (label_file_map_.empty()) {
            RCLCPP_WARN(this->get_logger(),
                        "[WARN] Label file '%s' not found or empty.", label_list_param_.c_str());
        }

        runtime_status_ = runtime_.LoadModel(
            model_dir_param_,
            drpai_data.address + DRPAI_MEM_OFFSET);

        if (!runtime_status_) {
            RCLCPP_ERROR(this->get_logger(),
                         "[ERROR] Failed to load model from '%s'.", model_dir_param_.c_str());
            return false;
        }
        RCLCPP_INFO(this->get_logger(), "[INFO] Model loaded: %s", model_dir_param_.c_str());
        return true;
    }

    /* -----------------------------------------------------------------------
     *  YOLOv3 post-processing
     * --------------------------------------------------------------------- */
    void post_process(float * floatarr)
    {
        det_.clear();

        float new_w, new_h;
        float correct_w = 1.f, correct_h = 1.f;
        if ((float)(MODEL_IN_W / correct_w) < (float)(MODEL_IN_H / correct_h)) {
            new_w = (float)MODEL_IN_W;
            new_h = correct_h * MODEL_IN_W / correct_w;
        } else {
            new_w = correct_w * MODEL_IN_H / correct_h;
            new_h = MODEL_IN_H;
        }

        for (int32_t n = 0; n < NUM_INF_OUT_LAYER; n++) {
            uint8_t num_grid   = num_grids[n];
            uint8_t anchor_off = 2 * NUM_BB * (NUM_INF_OUT_LAYER - (n + 1));

            for (int32_t b = 0; b < NUM_BB; b++) {
                for (int32_t y = 0; y < num_grid; y++) {
                    for (int32_t x = 0; x < num_grid; x++) {
                        int32_t offs = yolo_offset(n, b, y, x);
                        float tx = floatarr[offs];
                        float ty = floatarr[yolo_index(n, offs, 1)];
                        float tw = floatarr[yolo_index(n, offs, 2)];
                        float th = floatarr[yolo_index(n, offs, 3)];
                        float tc = floatarr[yolo_index(n, offs, 4)];

                        float cx = ((float)x + sigmoid(tx)) / (float)num_grid;
                        float cy = ((float)y + sigmoid(ty)) / (float)num_grid;
                        float bw = (float)exp(tw) * anchors[anchor_off + 2*b + 0] / (float)MODEL_IN_W;
                        float bh = (float)exp(th) * anchors[anchor_off + 2*b + 1] / (float)MODEL_IN_W;

                        cx = (cx - (MODEL_IN_W - new_w) / 2.f / MODEL_IN_W) / (new_w / MODEL_IN_W);
                        cy = (cy - (MODEL_IN_H - new_h) / 2.f / MODEL_IN_H) / (new_h / MODEL_IN_H);
                        bw *= (float)(MODEL_IN_W / new_w);
                        bh *= (float)(MODEL_IN_H / new_h);

                        cx = round(cx * DRPAI_IN_WIDTH);
                        cy = round(cy * DRPAI_IN_HEIGHT);
                        bw = round(bw * DRPAI_IN_WIDTH);
                        bh = round(bh * DRPAI_IN_HEIGHT);

                        float objectness = sigmoid(tc);
                        Box bb = {cx, cy, bw, bh};

                        float classes[NUM_CLASS];
                        for (int32_t i = 0; i < NUM_CLASS; i++)
                            classes[i] = sigmoid(floatarr[yolo_index(n, offs, 5 + i)]);

                        float   max_pred   = 0;
                        int32_t pred_class = -1;
                        for (int32_t i = 0; i < NUM_CLASS; i++) {
                            if (classes[i] > max_pred) { pred_class = i; max_pred = classes[i]; }
                        }

                        float prob = max_pred * objectness;
                        if (prob > TH_PROB) {
                            detection d = {bb, pred_class, prob};
                            det_.push_back(d);
                        }
                    }
                }
            }
        }

        filter_boxes_nms(det_, det_.size(), TH_NMS);

        /* Keep only the largest box per class */
        for (size_t i = 0; i < det_.size(); i++) {
            for (size_t j = 0; j < det_.size(); j++) {
                if (i == j) continue;
                if (det_[i].c != det_[j].c) continue;
                if (det_[i].prob == 0 || det_[j].prob == 0) continue;
                float ai = det_[i].bbox.h * det_[i].bbox.w;
                float aj = det_[j].bbox.h * det_[j].bbox.w;
                if (ai > aj) det_[j].prob = 0;
                else         det_[i].prob = 0;
            }
        }
    }

    /* -----------------------------------------------------------------------
     *  Draw bounding box on a BGRA mat, return gesture label
     * --------------------------------------------------------------------- */
    std::string draw_bounding_box(cv::Mat & bgra_frame)
    {
        if (det_.empty()) return "";

        uint32_t max_idx = 0;
        for (size_t i = 0; i < det_.size(); i++) {
            if (det_[i].prob >= det_[max_idx].prob) max_idx = i;
        }
        if (det_[max_idx].prob == 0) return "";

        std::string gesture = (det_[max_idx].c < (int)label_file_map_.size())
                              ? label_file_map_[det_[max_idx].c] : "unknown";

        int32_t x_min = (int)det_[max_idx].bbox.x - round((int)det_[max_idx].bbox.w / 2.);
        int32_t y_min = (int)det_[max_idx].bbox.y - round((int)det_[max_idx].bbox.h / 2.);
        int32_t x_max = (int)det_[max_idx].bbox.x + round((int)det_[max_idx].bbox.w / 2.) - 1;
        int32_t y_max = (int)det_[max_idx].bbox.y + round((int)det_[max_idx].bbox.h / 2.) - 1;

        x_min = std::max(1, x_min);  x_max = std::min(DRPAI_IN_WIDTH  - 2, x_max);
        y_min = std::max(1, y_min);  y_max = std::min(DRPAI_IN_HEIGHT - 2, y_max);

        cv::rectangle(bgra_frame, Point(x_min, y_min), Point(x_max, y_max),
                      Scalar(0, 255, 0, 255), BOX_THICKNESS);

        std::string label_text = "Gesture: " + gesture;
        cv::putText(bgra_frame, label_text, Point(HAND_STR_X, HAND_STR_Y),
                    FONT_HERSHEY_SIMPLEX, GESTURE_SCALE_SMALL,
                    Scalar(0, 0, 0, 255), (int)(1.5 * GESTURE_CHAR_THICKNESS));
        cv::putText(bgra_frame, label_text, Point(HAND_STR_X, HAND_STR_Y),
                    FONT_HERSHEY_SIMPLEX, GESTURE_SCALE_SMALL,
                    Scalar(0, 255, 255, 255), (int)GESTURE_CHAR_THICKNESS);

        return gesture;
    }

    /* -----------------------------------------------------------------------
     *  Full inference pipeline: pre-proc → DRP-AI → post-proc
     * --------------------------------------------------------------------- */
    bool run_inference(const cv::Mat & bgr_frame)
    {
        auto t0 = std::chrono::high_resolution_clock::now();

        /* Resize and convert HWC BGR -> CHW normalised float */
        cv::Mat frame1;
        cv::resize(bgr_frame, frame1, Size(MODEL_IN_W, MODEL_IN_H));

        vector<Mat> rgb_images;
        split(frame1, rgb_images);
        Mat m_flat_r = rgb_images[0].reshape(1, 1);
        Mat m_flat_g = rgb_images[1].reshape(1, 1);
        Mat m_flat_b = rgb_images[2].reshape(1, 1);
        Mat matArray[] = {m_flat_r, m_flat_g, m_flat_b};
        Mat frameCHW;
        hconcat(matArray, 3, frameCHW);
        frameCHW.convertTo(frameCHW, CV_32FC3);
        divide(frameCHW, 255.0, frameCHW);
        if (!frameCHW.isContinuous()) frameCHW = frameCHW.clone();

        auto t1 = std::chrono::high_resolution_clock::now();

        runtime_.SetInput(0, frameCHW.ptr<float>());
        auto t2 = std::chrono::high_resolution_clock::now();
        runtime_.Run(drpai_freq_);
        auto t3 = std::chrono::high_resolution_clock::now();

        auto t4 = std::chrono::high_resolution_clock::now();
        int32_t  output_num = runtime_.GetNumOutput();
        uint32_t size_count = 0;
        int      ret        = 0;

        for (int32_t i = 0; i < output_num; i++) {
            auto    output_buffer = runtime_.GetOutput(i);
            int64_t output_size   = std::get<2>(output_buffer);

            if (InOutDataType::FLOAT16 == std::get<0>(output_buffer)) {
                uint16_t * ptr = reinterpret_cast<uint16_t *>(std::get<1>(output_buffer));
                for (int j = 0; j < output_size; j++)
                    drpai_output_buf_[j + size_count] = float16_to_float32(ptr[j]);
            } else if (InOutDataType::FLOAT32 == std::get<0>(output_buffer)) {
                float * ptr = reinterpret_cast<float *>(std::get<1>(output_buffer));
                for (int j = 0; j < output_size; j++)
                    drpai_output_buf_[j + size_count] = ptr[j];
            } else {
                RCLCPP_ERROR(this->get_logger(), "[ERROR] Unsupported DRP-AI output data type.");
                ret = -1; break;
            }
            size_count += output_size;
        }
        if (ret != 0) return false;

        post_process(drpai_output_buf_);
        auto t5 = std::chrono::high_resolution_clock::now();

        pre_proc_time_  = std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count() / 1000.0f;
        inf_time_       = std::chrono::duration_cast<std::chrono::microseconds>(t3 - t2).count() / 1000.0f;
        post_proc_time_ = std::chrono::duration_cast<std::chrono::microseconds>(t5 - t4).count() / 1000.0f;
        total_time_     = pre_proc_time_ + inf_time_ + post_proc_time_;
        return true;
    }

    /* -----------------------------------------------------------------------
     *  Camera capture loop — runs in its own thread.
     *  Continuously grabs frames and stores the latest in latest_frame_.
     *  This prevents the inference timer from ever blocking on cap_ >> frame.
     * --------------------------------------------------------------------- */
    void capture_loop()
    {
        while (capture_running_) {
            cv::Mat frame;
            cap_ >> frame;
            if (frame.empty()) continue;

            {
                std::lock_guard<std::mutex> lock(frame_mutex_);
                latest_frame_ = frame;
                frame_ready_  = true;
            }
        }
    }

    /* -----------------------------------------------------------------------
     *  Timer callback: grab latest frame → infer → annotate → publish
     * --------------------------------------------------------------------- */
    void inference_callback()
    {
        /* --- Get the latest frame captured by the capture thread --- */
        cv::Mat bgr_frame;
        {
            std::lock_guard<std::mutex> lock(frame_mutex_);
            if (latest_frame_.empty()) return;  /* no frame at all yet */
            bgr_frame    = latest_frame_.clone();
            frame_ready_ = false;
        }

        /* --- Run DRP-AI inference on the BGR frame --- */
        if (!run_inference(bgr_frame)) {
            RCLCPP_ERROR(this->get_logger(), "[ERROR] DRP-AI inference failed.");
            return;
        }

        /* --- Convert BGR -> BGRA for drawing, matching darknet_drp_ros --- */
        cv::Mat out_image;
        cv::cvtColor(bgr_frame, out_image, cv::COLOR_BGR2BGRA);

        /* --- Annotate BGRA image with bounding box + label --- */
        std::string gesture = draw_bounding_box(out_image);

        /* --- Publish gesture label --- */
        auto det_msg = std_msgs::msg::String();
        det_msg.data = gesture;
        pub_detection_->publish(det_msg);

        /* --- Publish detection image as JPEG compressed (every Nth frame) ----------
         * Raw BGR8 640x480 = ~7.4 Mbit/frame. At 10fps that is ~74 Mbps which
         * saturates a 100Mbps link and causes DDS to drop packets.
         * JPEG at quality 80 is ~150-200 KB/frame = ~15 Mbps at 10fps.
         * ------------------------------------------------------------------ */
        if (++image_frame_counter_ % image_publish_every_ == 0) {
            cv::Mat bgr;
            cv::cvtColor(out_image, bgr, cv::COLOR_BGRA2BGR);

            std::vector<uchar> jpeg_buf;
            std::vector<int>   jpeg_params = {cv::IMWRITE_JPEG_QUALITY, jpeg_quality_};
            cv::imencode(".jpg", bgr, jpeg_buf, jpeg_params);

            sensor_msgs::msg::CompressedImage comp_msg;
            comp_msg.header.stamp    = this->now();
            comp_msg.header.frame_id = "detection_image";
            comp_msg.format          = "jpeg";
            comp_msg.data            = jpeg_buf;
            pub_image_->publish(comp_msg);
        }

        /* --- Publish timing diagnostics as JSON --- */
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(2)
            << "{\"pre_proc_ms\":"  << pre_proc_time_
            << ",\"inference_ms\":" << inf_time_
            << ",\"post_proc_ms\":" << post_proc_time_
            << ",\"total_ms\":"     << total_time_
            << ",\"gesture\":\""    << gesture
            << "\"}";
        auto timing_msg = std_msgs::msg::String();
        timing_msg.data = oss.str();
        pub_timing_->publish(timing_msg);

        RCLCPP_DEBUG(this->get_logger(),
                     "Gesture: '%s'  total=%.1f ms  (pre=%.1f inf=%.1f post=%.1f)",
                     gesture.c_str(), total_time_,
                     pre_proc_time_, inf_time_, post_proc_time_);
    }

    /* ---- Member variables ---- */
    MeraDrpRuntimeWrapper runtime_;
    bool                  runtime_status_{false};
    int                   drpai_fd_{-1};
    float                 drpai_output_buf_[INF_OUT_SIZE];
    vector<detection>     det_;
    vector<string>        label_file_map_;

    int         drpai_freq_;
    std::string model_dir_param_;
    std::string label_list_param_;
    std::string camera_device_;

    VideoCapture       cap_;             /* V4L2 direct — no GStreamer */

    /* Camera capture thread */
    std::thread        capture_thread_;
    std::atomic<bool>  capture_running_{false};
    cv::Mat            latest_frame_;
    std::mutex         frame_mutex_;
    bool               frame_ready_{false};

    float pre_proc_time_{0}, inf_time_{0}, post_proc_time_{0}, total_time_{0};

    /* Image publish throttle */
    int      jpeg_quality_{80};
    int      image_publish_every_{1};
    uint32_t image_frame_counter_{0};

    rclcpp::Publisher<std_msgs::msg::String>::SharedPtr    pub_detection_;
    rclcpp::Publisher<sensor_msgs::msg::CompressedImage>::SharedPtr  pub_image_;
    rclcpp::Publisher<std_msgs::msg::String>::SharedPtr    pub_timing_;
    rclcpp::TimerBase::SharedPtr                           timer_;
};

/*========================================================
 *  main
 *========================================================*/
int main(int argc, char ** argv)
{
    rclcpp::init(argc, argv);
    try {
        auto node = std::make_shared<HandGestureRecognitionNode>();
        rclcpp::spin(node);
    } catch (const std::exception & e) {
        RCLCPP_FATAL(rclcpp::get_logger("hand_gesture_recognition"),
                     "Node crashed: %s", e.what());
        rclcpp::shutdown();
        return 1;
    }
    rclcpp::shutdown();
    return 0;
}
