/**
 * @file camera.hpp
 * @brief
 *
 * @copyright Copyright (C) 2023 Renesas Electronics Corporation. All rights reserved.
 * @copyright Copyright (C) 2024 IMD Technologies Ltd.
 *
 */

#pragma once

#include <linux/videodev2.h>

#include <memory>

#include "define.h"
#include "dma_buffer.hpp"


#define WAYLANDBUF (IMAGE_OUTPUT_WIDTH * IMAGE_OUTPUT_HEIGHT * IMAGE_CHANNEL_BGR * WL_BUF_NUM)
#define CAPTUREBUF (CAM_IMAGE_WIDTH * CAM_IMAGE_HEIGHT * CAM_IMAGE_CHANNEL_YUY2)
#define DRPAIBUF (CAM_IMAGE_WIDTH * CAM_IMAGE_HEIGHT * CAM_IMAGE_CHANNEL_YUY2)



/**
 * @brief
 */
class Camera {
 public:

  enum class CameraSource : uint8_t {
    CRU0,
    CRU1,
    UVC0,
    UVC1,
  };

  using SharedPtr = std::shared_ptr<Camera>;

  /**
   * @brief Construct a new Camera object
   * @param camera_source
   * @param width Capture width, in pixels
   * @param height Capture height, in pixels
   */
  Camera(CameraSource camera_source, uint32_t width, const uint32_t height);

  /**
   * @brief Destructor
   */
  ~Camera() { closeCamera(); }

  /**
   * @brief Starts the capture device
   * @return
   */
  bool startCamera();

  /**
   * @brief Waits for an image to be captured by the device
   * @return
   */
  bool captureImage();

  /**
   * @brief (Re)queue the active capture buffer
   * @return
   */
  bool queueCaptureBuffer();

  /**
   * @brief Returns a pointer to the capture buffer data
   * @return
   */
  uint8_t *getCaptureBufferData() {
    return reinterpret_cast<uint8_t *>(capture_dma_buffers_[active_capture_buffer_.index]->mem());
  }

  /**
   * @brief Get the capture buffer size
   * @return Buffer size in bytes
   */
  uint32_t getCaptureBufferSize() { return capture_dma_buffers_[active_capture_buffer_.index]->size(); }

 private:
  /**
   * @brief Searches for, and opens, a compatible capture device
   * @return
   */
  bool openCaptureDevice();

  /**
   * @brief Sets the capture format and frame interval
   * @return
   */
  bool setCaptureFormat();

  /**
   * @brief Requests the V4L2 capture buffers
   * @return
   */
  bool requestCaptureBuffers();

  /**
   * @brief
   */
  bool startCapture();

  /**
   * @brief
   */
  bool stopCapture();

  /**
   * @brief
   */
  void closeCamera();

  /// @brief Camera Source
  CameraSource cam_src_;

  /// @brief Capture width, in pixels
  uint32_t capture_width_;

  /// @brief Capture height, in pixels
  uint32_t capture_height_;

  /// @brief The number of capture buffers
  uint32_t number_of_capture_buffers_{CAP_BUF_NUM};

  /// @brief File descriptor for the V4L2 device
  int fd_;

  /// @brief DMA buffer for each V4L2 capture buffer
  std::vector<DmaBuffer::SharedPtr> capture_dma_buffers_;

  /// @brief The active V4L2 capture buffer
  v4l2_buffer active_capture_buffer_;

  /// @todo Move these elsewhere
};
