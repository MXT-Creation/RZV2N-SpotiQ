/**
 * @file box.hpp
 * @brief
 * @copyright Copyright (c) 2023 Renesas Electronics Corporation. All rights reserved.
 */

#pragma once

#include <memory>
#include <vector>

struct Box {
  float x;
  float y;
  float w;
  float h;
};

struct Detection {
  Box bbox;
  int c;
  float prob;
};

using Detections = std::shared_ptr<std::vector<Detection>>;

/**
 * @brief Perform NMS to remove overlapping detections
 * @param detections Vector of Detection objects
 * @param th_nms NMS threshold
 */
void filter_boxes_nms(Detections detections, float th_nms);
