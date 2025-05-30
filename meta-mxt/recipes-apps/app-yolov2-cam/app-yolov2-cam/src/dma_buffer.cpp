/**
 * @file dma_buffer.cpp
 * @brief
 * @copyright Copyright (c) 2024
 */

#include "dma_buffer.hpp"

#ifdef __cplusplus
extern "C" {
#endif
// clang-format off
// Disable clang-format to ensure stddef.h is included first, as the Renesas headers aren't standalone
#include <stddef.h>
#include <mmngr_buf_user_public.h>
#include <mmngr_user_public.h>
// clang-format on
#ifdef __cplusplus
}
#endif

#include <cstring>
#include <iostream>
#include <stdexcept>

DmaBuffer::DmaBuffer(const uint32_t size, const uint32_t count) {
  std::cout << "[DmaBuffer] allocating buffer" << std::endl;
  MMNGR_ID id;
  uint32_t phard_addr;
  void *puser_virt_addr;
  int m_dma_fd;

  size_ = (size * count);
  count_ = count;
  mmngr_alloc_in_user_ext(&id, size_, &phard_addr, &puser_virt_addr, MMNGR_VA_SUPPORT_CACHED, NULL);
  memset((void *)puser_virt_addr, 0, size_);
  index_ = id;
  mem_ = (void *)puser_virt_addr;
  physical_address_ = phard_addr;
  if (!mem_) {
    throw std::runtime_error("Failed to allocate DMA buffer");
  }
  mmngr_export_start_in_user_ext(&id, size_, phard_addr, &m_dma_fd, NULL);
  fd_ = m_dma_fd;
}

DmaBuffer::~DmaBuffer() {
  std::cout << "[DmaBuffer] deallocating buffer" << std::endl;
  mmngr_free_in_user_ext(index_);
}

bool DmaBuffer::flush() { return (0 == mmngr_flush(index_, 0, size_)); }
