/**
 * @file image.cpp
 * @brief
 * @copyright Copyright (c) 2023 Renesas Electronics Corporation. All rights reserved.
 * @copyright Copyright (c) 2024 IMDT Technologies Ltd.
 */

#include "image.hpp"

#include <opencv2/opencv.hpp>

Image::Image(ImageDimensions input_dimensions, ImageDimensions output_dimensions, DmaBuffer::SharedPtr wayland_buffer) {
  input_dimensions_ = input_dimensions;
  output_dimensions_ = output_dimensions;

  const uint32_t output_size = output_dimensions.sizeBytes();
  auto *const mem = static_cast<uint8_t *>(wayland_buffer->mem());

  for (uint32_t i = 0; i < wayland_buffer->count(); i++) {
    wayland_buffers_.push_back(mem + (i * output_size));
  }

  // Working buffer for YUY2 to BGR conversion
  image_conversion_buffer_.resize(input_dimensions_.width * input_dimensions_.height * output_dimensions_.channels);
}

void Image::copyCameraDataToWaylandBuffer(const uint8_t *capture_buffer_data, const uint32_t capture_buffer_size) {
  size_t next_buffer_id = ++active_buffer_id_ % wayland_buffers_.size();
  active_buffer_id_ = next_buffer_id;
  memcpy(wayland_buffers_[active_buffer_id_], capture_buffer_data, capture_buffer_size);
}

/**
 *
 */
void Image::convertToBGR() {
  uint8_t *yuyv_pixel_data = wayland_buffers_[active_buffer_id_];

  cv::Mat source_image(input_dimensions_.height, 
                       input_dimensions_.width,  
                       CV_8UC2, 
                       static_cast<void *>(yuyv_pixel_data));

  cv::Mat bgr_image(output_dimensions_.height,   
                    output_dimensions_.width, 
                    CV_8UC3, 
                    static_cast<void *>(image_conversion_buffer_.data()));

  cv::cvtColor(source_image, bgr_image, cv::COLOR_YUV2BGR_YUYV);
  memcpy(wayland_buffers_[active_buffer_id_], image_conversion_buffer_.data(),
         input_dimensions_.width * input_dimensions_.height * output_dimensions_.channels /* img_w * img_h * out_c */);
}

/**
 *
 */
void Image::resizeImage(const uint32_t output_width) {
  if (output_width == input_dimensions_.width) {
    return;
  }

  cv::Mat source_image(input_dimensions_.height, input_dimensions_.width, CV_8UC3, wayland_buffers_[active_buffer_id_]);
  cv::Mat resized_image;

  const float scale_factor = static_cast<float>(output_width) / input_dimensions_.width;
  cv::resize(source_image, resized_image, cv::Size(), scale_factor, scale_factor);
  memcpy(wayland_buffers_[active_buffer_id_], resized_image.data, output_dimensions_.sizeBytes());
}

/**
 *
 */
void Image::drawText(const std::string& text, const TextAlignment alignment, const int x, const int y,
                     const float scale, uint32_t color) {
  int thickness = CHAR_THICKNESS;

  uint8_t r = (color >> 16) & 0x0000FF;
  uint8_t g = (color >> 8) & 0x0000FF;
  uint8_t b = color & 0x0000FF;
  int ptx = x;
  int pty = y;

  cv::Mat bgr_image(output_dimensions_.height, output_dimensions_.width, CV_8UC3, wayland_buffers_[active_buffer_id_]);

  int baseline = 0;
  cv::Size size = cv::getTextSize(text.c_str(), cv::FONT_HERSHEY_SIMPLEX, scale, thickness + 2, &baseline);

  if (alignment == TextAlignment::kRightAligned) {
    ptx = output_dimensions_.width - (size.width + x);
  }

  cv::putText(bgr_image, text.c_str(), cv::Point(ptx, pty), cv::FONT_HERSHEY_SIMPLEX, scale,
              cv::Scalar(0x00, 0x00, 0x00), thickness + 2);
  cv::putText(bgr_image, text.c_str(), cv::Point(ptx, pty), cv::FONT_HERSHEY_SIMPLEX, scale, cv::Scalar(b, g, r),
              thickness);
}

/**
 * @todo
 */
void Image::drawRectangle(const int x, const int y, const int w, const int h, const std::string& text, uint32_t color) {
  int32_t x_min = x - round(w / 2.);
  int32_t y_min = y - round(h / 2.);
  int32_t x_max = x + round(w / 2.) - 1;
  int32_t y_max = y + round(h / 2.) - 1;

  int32_t image_width = static_cast<int32_t>(input_dimensions_.width) - 2;
  int32_t image_height = static_cast<int32_t>(input_dimensions_.height) - 2;

  x_min = x_min < 1 ? 1 : x_min;
  x_max = (image_width < x_max) ? image_width : x_max;
  y_min = y_min < 1 ? 1 : y_min;
  y_max = (image_height < y_max) ? image_height : y_max;

  drawRectangleWithLabel(text.c_str(), TextAlignment::kLeftAligned, x_min, y_min, x_max, y_max, CHAR_SCALE_FONT, color);

  return;
}

/*
 * Private
 */

/**
 *
 */
void Image::drawRectangleWithLabel(const std::string& text, const TextAlignment alignment, uint32_t x_min,
                                   uint32_t y_min, uint32_t x_max, uint32_t y_max, float scale, uint32_t color) {
  uint8_t thickness = CHAR_THICKNESS;

  uint8_t r = (color >> 16) & 0x0000FF;
  uint8_t g = (color >> 8) & 0x0000FF;
  uint8_t b = color & 0x0000FF;

  cv::Mat bgr_image(output_dimensions_.height, output_dimensions_.width, CV_8UC3, wayland_buffers_[active_buffer_id_]);

  int baseline = 0;
  cv::rectangle(bgr_image, cv::Point(x_min, y_min), cv::Point(x_max, y_max), cv::Scalar(b, g, r), BOX_LINE_SIZE);

  cv::Size size = cv::getTextSize(text, cv::FONT_ITALIC, scale, thickness + 2, &baseline);

  int ptx = x_min;
  int pty = y_min;
  if (alignment == TextAlignment::kRightAligned) {
    ptx = x_max - size.width;
    pty = y_min;
  }

  cv::rectangle(bgr_image, cv::Point(ptx - BOX_LINE_SIZE + 1, pty - BOX_HEIGHT_OFFSET),
                cv::Point(ptx + size.width, pty), cv::Scalar(b, g, r), cv::FILLED);

  cv::putText(bgr_image, text, cv::Point(ptx, pty - BOX_TEXT_HEIGHT_OFFSET), cv::FONT_ITALIC, scale,
              cv::Scalar(0x00, 0x00, 0x00), thickness);
}

std::string Image::getDominantColor(int x, int y, int w, int h) {
    std::string dominant_channel;
    cv::Mat bgr_image(output_dimensions_.height, 
                      output_dimensions_.width, 
                      CV_8UC3, 
                      wayland_buffers_[active_buffer_id_]);

    int32_t x_min = x - static_cast<int32_t>(std::round(w / 2.0));
    int32_t y_min = y - static_cast<int32_t>(std::round(h / 2.0));
    int32_t x_max = x + static_cast<int32_t>(std::round(w / 2.0)) - 1;
    int32_t y_max = y + static_cast<int32_t>(std::round(h / 2.0)) - 1;

    int32_t image_width  = static_cast<int32_t>(input_dimensions_.width) - 2;
    int32_t image_height = static_cast<int32_t>(input_dimensions_.height) - 2;

    x_min = std::max(x_min, 1);
    x_max = std::min(x_max, image_width);
    y_min = std::max(y_min, 1);
    y_max = std::min(y_max, image_height);

    // Extract the region of interest (ROI) from the image
    cv::Rect roi(x_min, y_min, x_max - x_min + 1, y_max - y_min + 1);

    if (roi.x >= 0 && roi.y >= 0 && roi.width > 0 && roi.height > 0 &&
        roi.x + roi.width <= bgr_image.cols && roi.y + roi.height <= bgr_image.rows)
    {
        cv::Mat cropped_image = bgr_image(roi);

        // Calculate the average (mean) color in the cropped image
        cv::Scalar mean_color = cv::mean(cropped_image);
        double blue  = mean_color[0];
        double green = mean_color[1];
        double red   = mean_color[2];

        // Define a threshold for how close red and green should be 
        // to call the color “Yellow.” You may need to tweak this.
        const double closeThreshold = 20.0;

        // Simple logic for detecting red, green, blue, or yellow
        // 1) Check if red and green are both significantly larger than blue
        //    and close enough to each other -> "Yellow"
        // 2) Otherwise, pick whichever channel is largest -> "Red"/"Green"/"Blue"
        if ((red > blue) && (green > blue)) {
            // Red and green both dominate over blue. Could be yellow, red, or green.
            if (std::fabs(red - green) < closeThreshold) {
                dominant_channel = "Yellow";
            } else if (red > green) {
                dominant_channel = "Red";
            } else {
                dominant_channel = "Green";
            }
        } else {
            // Otherwise, pick the largest channel
            if (blue >= green && blue >= red) {
                dominant_channel = "Blue";
            } else if (green >= red) {
                dominant_channel = "Green";
            } else {
                dominant_channel = "Red";
            }
        }
    }
    else {
        std::cerr << "ROI is invalid or out of bounds!" << std::endl;
    }

    return dominant_channel;
}