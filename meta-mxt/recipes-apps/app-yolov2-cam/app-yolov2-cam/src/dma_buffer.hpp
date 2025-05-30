/**
 * @file dma_buffer.hpp
 * @brief
 * @copyright Copyright (c) 2024 IMD Technologies Ltd
 */

#pragma once

#include <cstdint>
#include <memory>

/**
 * @brief Wrapper for a DMA buffer created by the mmngr driver
 */
class DmaBuffer {
 public:
  using SharedPtr = std::shared_ptr<DmaBuffer>;

  /**
   * @brief Allocates a new DMA buffer, equal to (size * count) bytes
   * @param size Buffer size, in bytes
   * @param count Number of buffers
   */
  explicit DmaBuffer(const uint32_t size, const uint32_t count = 1);

  DmaBuffer(const DmaBuffer&) = delete;
  DmaBuffer& operator=(const DmaBuffer&) = delete;

  /**
   * @brief Deallocates the DMA buffer
   */
  ~DmaBuffer();

  /**
   * @brief Flush the contents of a DMA buffer
   * @return
   */
  bool flush();

  /**
   * @brief Returns the buffer's index
   * @return
   */
  uint32_t index() const { return index_; }

  /**
   * @brief Returns the file descriptor for the buffer
   * @details Used when assiging the DMA buffer to a V4L buffer
   * @return
   */
  int fd() const { return fd_; }

  /**
   * @brief Returns the buffer's total size
   * @return Total size in bytes
   */
  uint32_t size() const { return size_; }

  /**
   * @brief Returns the number of buffers
   * @return Number of buffers
   */
  uint32_t count() const { return count_; }

  /**
   * @brief Returns the physical address for the buffer
   * @return Physical address in memory
   */
  uint32_t physicalAddress() const { return physical_address_; }

  /**
   * @brief Returns a pointer to the DMA buffer contents
   * @return
   */
  void *mem() const { return mem_; }

 private:
  /// @brief DMA buffer index
  uint32_t index_;

  /// @brief File descriptor
  int fd_;

  /// @brief Total size in bytes
  uint32_t size_;

  uint32_t count_;

  /// @brief Physical address
  uint32_t physical_address_;

  /// @brief Pointer to the data
  void *mem_;
};
