/**
 * @file image.hpp
 * @brief
 * @copyright Copyright (c) 2023 Renesas Electronics Corporation. All rights reserved.
 * @copyright Copyright (c) 2024 IMDT Technologies Ltd.
 */

#pragma once

#include <memory>

#include "ascii.h"
#include "define.h"
#include "dma_buffer.hpp"


struct ImageDimensions {
  uint32_t width;
  uint32_t height;
  uint32_t channels;

  uint32_t sizeBytes() { return (width * height * channels); }
};

enum class TextAlignment { kLeftAligned, kRightAligned };

/**
 * @brief Copies camera data to the Wayland buffer, and annotates the images
 */
class Image {
 public:
  using SharedPtr = std::shared_ptr<Image>;

  /**
   * @brief Initialises the image object
   * @param input_dimensions Input dimensions (w, h, c)
   * @param output_dimensions Output dimensions (w, h, c)
   * @param wayland_buffer Wayland DMA buffer
   */
  Image(ImageDimensions input_dimensions, ImageDimensions output_dimensions, DmaBuffer::SharedPtr wayland_buffer);

  Image(const Image&) = delete;
  Image& operator=(const Image&) = delete;

  /**
   * @brief Copies the data from a capture buffer into the Wayland buffer
   * @param capture_buffer_data Pointer to the capture buffer data
   * @param capture_buffer_size Size of the capture buffer
   */
  void copyCameraDataToWaylandBuffer(const uint8_t *capture_buffer_data, uint32_t capture_buffer_size);

  /**
   * @brief Converts the YUYV camera data into BGR
   */
  void convertToBGR();

  /**
   * @brief Resizes the output image, based on the new width
   * @param output_width The new output width
   */
  void resizeImage(const uint32_t output_width);

  /**
   * @brief Returns the active buffer ID
   * @return Buffer ID/index
   */
  size_t getActiveBufferId() const { return active_buffer_id_; }

  /**
   * @brief Returns a pointer to the Wayland buffer data
   * @param buffer_id Buffer ID/index
   * @return Pointer to the relevant buffer
   */
  uint8_t *getWaylandBufferData(size_t buffer_id) const { return wayland_buffers_.at(buffer_id); }

  /**
   * @brief Draws a text string on the image buffer
   * @param text
   * @param alignment
   * @param x
   * @param y
   * @param size
   * @param color
   */
  void drawText(const std::string& text, const TextAlignment alignment, const int x, const int y, const float size,
                uint32_t color);

  /**
   * @brief Draws a rectangle with a text label on the image buffer
   * @param x
   * @param y
   * @param w
   * @param h
   * @param text
   * @param color
   */
  void drawRectangle(const int x, const int y, const int w, const int h, const std::string& text, uint32_t color);

  ImageDimensions getInputDimensions()  const noexcept { return input_dimensions_;  }
  ImageDimensions getOutputDimensions() const noexcept { return output_dimensions_; }

std::string getDominantColor(int x, int y, int w, int h);
 private:
  /**
   * @brief Draws a rectangle with a text label on the image buffer
   * @param text
   * @param alignment
   * @param x_min
   * @param y_min
   * @param x_max
   * @param y_max
   * @param scale
   * @param color
   */
  void drawRectangleWithLabel(const std::string& text, const TextAlignment alignment, uint32_t x_min, uint32_t y_min,
                              uint32_t x_max, uint32_t y_max, float scale, uint32_t color);

  /// @brief Input dimensions (w, h, c)
  ImageDimensions input_dimensions_;

  /// @brief Output dimensions (w, h, c)
  ImageDimensions output_dimensions_;

  /// @brief Vector of buffer pointers; these point to different parts of the Wayland DMA buffer
  std::vector<uint8_t *> wayland_buffers_;

  /// @brief Working buffer for the YUY2 to BGR conversion
  std::vector<uint8_t> image_conversion_buffer_;

  /// @brief The active buffer ID/index
  size_t active_buffer_id_{0};
};
