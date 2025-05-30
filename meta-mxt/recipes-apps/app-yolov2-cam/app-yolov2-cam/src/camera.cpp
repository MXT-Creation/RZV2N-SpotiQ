/**
 * @file camera.cpp
 * @brief
 *
 * @copyright Copyright (C) 2023 Renesas Electronics Corporation. All rights reserved.
 * @copyright Copyright (C) 2024 IMD Technologies Ltd.
 *
 */

#include "camera.hpp"

#include <exception>
#include <iostream>
#include <iterator>
#include <memory>
#include "media_device.hpp"


/*****************************************
 * Function Name : format
 * Description   : Format specification.
 * Arguments     : fmt = an object that represents the format string.
 *                 args = arguments to be formatted
 * Return value  : A string object holding the formatted result.
 ******************************************/
template <typename... Args>
static std::string format(const std::string& fmt, Args... args) {
  size_t len = std::snprintf(nullptr, 0, fmt.c_str(), args...);
  std::vector<char> buf(len + 1);
  std::snprintf(&buf[0], len + 1, fmt.c_str(), args...);
  return std::string(&buf[0], &buf[0] + len);
}

  /**
   * @brief
   * @param fd
   * @param request
   * @param arg
   * @return
   */
static bool xioctl(int fd, unsigned long request, void *arg) {
  int r;
  do {
    r = ioctl(fd, request, arg);
  } while ((-1 == r) && (EINTR == errno));
  return (r != -1);
}


Camera::Camera(CameraSource camera_source, uint32_t width, const uint32_t height) {
  cam_src_        = camera_source;
  capture_width_  = width;
  capture_height_ = height;
}

bool Camera::startCamera() {
  MediaDevice media_dev{static_cast<int>(cam_src_)};
  if (!media_dev.isOpen()) {
    std::cerr << "Failed to open /dev/media" << static_cast<int>(cam_src_) << std::endl;
    return false;
  }

  // Only need to configure the pipeline for the MIPI Cameras
  if (cam_src_ < CameraSource::UVC0) 
  {
    std::vector<EntityConfig> entity_configs(3);

    EntityConfig& isp_config = entity_configs[0];
    isp_config.type          = EntityType::VideoIsp;
    isp_config.source_pad    = EntityPad{EntityPad::PadType::SOURCE, 0U, "YUYV", {capture_width_, capture_height_}};

    EntityConfig& csi_config = entity_configs[1];
    csi_config.type          = EntityType::CsiMipiReceiver;
    csi_config.sink_pad      = EntityPad{EntityPad::PadType::SINK,   0U, "YUYV", {capture_width_, capture_height_}};
    csi_config.source_pad    = EntityPad{EntityPad::PadType::SOURCE, 1U, "YUYV", {capture_width_, capture_height_}};

    EntityConfig& cru_config = entity_configs[2];
    cru_config.type          = EntityType::VideoCapture;
    cru_config.sink_pad      = EntityPad{EntityPad::PadType::SINK,   0U, "YUYV", {capture_width_, capture_height_}};
    cru_config.source_pad    = EntityPad{EntityPad::PadType::SOURCE, 1U, "YUYV", {capture_width_, capture_height_}};

    media_dev.configurePipeline(entity_configs);
  }

  if (!openCaptureDevice()) {
    std::cerr << "Failed to open capture device" << std::endl;
    return false;
  }

  if (!setCaptureFormat()) {
    std::cerr << "Failed to set the capture format" << std::endl;
    return false;
  }

  if (!requestCaptureBuffers()) {
    std::cerr << "Failed to request capture buffers" << std::endl;
    return false;
  }

  for (uint32_t n = 0; n < number_of_capture_buffers_; n++) {
    capture_dma_buffers_.push_back(std::make_shared<DmaBuffer>(CAPTUREBUF));

    memset(&active_capture_buffer_, 0, sizeof(active_capture_buffer_));
    active_capture_buffer_.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    active_capture_buffer_.memory = V4L2_MEMORY_DMABUF;
    active_capture_buffer_.index  = n;
    active_capture_buffer_.m.fd   = capture_dma_buffers_[n]->fd();
    active_capture_buffer_.length = capture_dma_buffers_[n]->size();

    if (!xioctl(fd_, VIDIOC_QBUF, &active_capture_buffer_)) {
      return false;
    }
  }

  if (!startCapture()) {
    return false;
  }

  return true;
}

bool Camera::captureImage() {
  fd_set fds;
  FD_ZERO(&fds);
  FD_SET(fd_, &fds);

  // Wait for a new frame to become available
  while (1) {
    int ret = select(fd_ + 1, &fds, NULL, NULL, NULL);
    if (0 > ret) {
      if (EINTR == errno) continue;
      return false;
    }
    break;
  }

  // Dequeue the capture buffer
  if (!xioctl(fd_, VIDIOC_DQBUF, &active_capture_buffer_)) {
    return false;
  }

  if (!capture_dma_buffers_[active_capture_buffer_.index]->flush()) {
    return false;
  }

  return true;
}

bool Camera::queueCaptureBuffer() { return xioctl(fd_, VIDIOC_QBUF, &active_capture_buffer_); }

/*
 * Private
 */

bool Camera::openCaptureDevice() {
  MediaDevice media_dev{static_cast<int>(cam_src_)};
  if (!media_dev.isOpen()) 
  {
    std::cerr << "Failed to open /dev/media" << static_cast<int>(cam_src_) << std::endl;
    return false;
  }

  std::string capture_dev_name{media_dev.getVideoCaptureDevice()};
  if (capture_dev_name.length() == 0) return false;
  
  int fd = ::open(capture_dev_name.c_str(), O_RDWR);
  if (fd != -1) 
  {
    fd_ = fd;
    return true;
  }
  return false;
}

bool Camera::setCaptureFormat() {
  struct v4l2_format fmt;
  memset(&fmt, 0, sizeof(fmt));

  fmt.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  fmt.fmt.pix.width       = capture_width_;
  fmt.fmt.pix.height      = capture_height_;
  fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
  fmt.fmt.pix.field       = V4L2_FIELD_NONE;

  if (!xioctl(fd_, VIDIOC_S_FMT, &fmt)) {
    std::cerr << "[ERROR] VIDIOC_S_FMT ioctl failed" << std::endl;
    return false;
  }

  // VIDIOC_S_PARM IOCTL not supported by rzg2l_cru driver 
  // So it will throw an error but we just ignore it
  struct v4l2_streamparm parm;
  memset(&parm, 0, sizeof(parm));

  parm.type                                  = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  parm.parm.capture.timeperframe.numerator   = 1;
  parm.parm.capture.timeperframe.denominator = 30;

  if (ioctl(fd_, VIDIOC_S_PARM, &parm) < 0) {
    std::cerr << "[ERROR] VIDIOC_S_PARM ioctl failed" << std::endl;
    perror("VIDIOC_S_PARM");
  }

  return true;
}

bool Camera::requestCaptureBuffers() {
  struct v4l2_requestbuffers req;
  memset(&req, 0, sizeof(req));

  req.count   = number_of_capture_buffers_;
  req.type    = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  req.memory  = V4L2_MEMORY_DMABUF;

  if (!xioctl(fd_, VIDIOC_REQBUFS, &req)) {
    std::cerr << "[ERROR] VIDIOC_REQBUFS ioctl failed" << std::endl;
    return false;
  }

  return true;
}

bool Camera::startCapture() {
  int type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  return xioctl(fd_, VIDIOC_STREAMON, &type);
}

bool Camera::stopCapture() {
  int type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  return xioctl(fd_, VIDIOC_STREAMOFF, &type);
}

void Camera::closeCamera() {
  if (!stopCapture()) {
    return;
  }

  ::close(fd_);
}
