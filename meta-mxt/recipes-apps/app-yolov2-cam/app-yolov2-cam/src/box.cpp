/**
 * @file box.cpp
 * @brief
 * @copyright Copyright (c) 2023 Renesas Electronics Corporation. All rights reserved.
 */

#include "box.hpp"

/**
 * @brief Function to compute the overlapped data between coordinate x with size w
 * @param x1 1-dimensional coordinate of first line
 * @param w1 size of fist line
 * @param x2 1-dimensional coordinate of second line
 * @param w2 size of second line
 * @return overlapped line size
 */
static float overlap(float x1, float w1, float x2, float w2) {
  float l1 = x1 - w1 / 2;
  float l2 = x2 - w2 / 2;
  float left = l1 > l2 ? l1 : l2;
  float r1 = x1 + w1 / 2;
  float r2 = x2 + w2 / 2;
  float right = r1 < r2 ? r1 : r2;
  return right - left;
}

/**
 * @brief Function to compute the area of intersection of Box a and b
 * @param a First box
 * @param b Second box
 * @return Area of intersection
 */
static float box_intersection(Box a, Box b) {
  float w = overlap(a.x, a.w, b.x, b.w);
  float h = overlap(a.y, a.h, b.y, b.h);
  if (w < 0 || h < 0) {
    return 0;
  }
  float area = w * h;
  return area;
}

/**
 * @brief Function to compute the area of union of Box a and b
 * @param a First box
 * @param b Second box
 * @return Area of union
 */
static float box_union(Box a, Box b) {
  float i = box_intersection(a, b);
  float u = a.w * a.h + b.w * b.h - i;
  return u;
}

/**
 * @brief Function to compute the Intersection over Union (IoU) of Box a and b
 * @param a First box
 * @param b Second box
 * @return IoU
 */
static float box_iou(Box a, Box b) { return box_intersection(a, b) / box_union(a, b); }

void filter_boxes_nms(Detections detections, float th_nms) {
  const int count = detections->size();
  for (int i = 0; i < count; i++) {
    Box a = (*detections)[i].bbox;
    for (int j = 0; j < count; j++) {
      if (i == j) {
        continue;
      }
      if ((*detections)[i].c != (*detections)[j].c) {
        continue;
      }
      Box b = (*detections)[j].bbox;
      float b_intersection = box_intersection(a, b);
      if ((box_iou(a, b) > th_nms) || (b_intersection >= a.h * a.w - 1) || (b_intersection >= b.h * b.w - 1)) {
        if ((*detections)[i].prob > (*detections)[j].prob) {
          (*detections)[j].prob = 0;
        } else {
          (*detections)[i].prob = 0;
        }
      }
    }
  }
}
