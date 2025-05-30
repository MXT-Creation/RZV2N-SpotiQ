/**
 * @file drp_ai_accelerator.cpp
 * @brief
 * @copyright Copyright (c) 2024 IMD Technologies
 */

#include "drp_ai_accelerator.hpp"

#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include <iostream>
#include <stdexcept>
#include <vector>

#include "define.h"

/**
 *
 */
DrpAiAccelerator::DrpAiAccelerator() {
  fd_ = ::open("/dev/drpai0", O_RDWR);
  if (fd_ < 0) {
    throw std::runtime_error("Failed to open DRP-AI device");
  }

  if (::ioctl(fd_, DRPAI_GET_DRPAI_AREA, &drpai_reserved_memory_) != 0) {
    throw std::runtime_error("Failed to get DRP-AI memory area");
  }
}

/**
 *
 */
DrpAiAccelerator::~DrpAiAccelerator() { ::close(fd_); }

/**
 *
 */
bool DrpAiAccelerator::loadObjectFiles(const std::string& object_directory) {
  object_directory_ = object_directory;

  drpai_address_map_.start_address = drpai_reserved_memory_.address;

  if (!readAddressMap()) {
    return false;
  }

  if (!loadParameterInfo()) {
    return false;
  }

  if (!loadDataObjects()) {
    std::cerr << "Failed to load data objects" << std::endl;
    return false;
  }

  return true;
}

/**
 *
 */
bool DrpAiAccelerator::setClockDividers(const uint32_t drp_clock_divider, const uint32_t ai_mac_clock_divider) {
  if ((drp_clock_divider >= 2) && (drp_clock_divider <= 29)) {
    if (ioctl(getFd(), DRPAI_SET_DRP_MAX_FREQ, &drp_clock_divider) != 0) {
      std::cerr << "[ERROR] Failed to set DRP clock divider" << std::endl;
      return false;
    }
  } else {
    std::cerr << "[ERROR] Invalid DRP clock divider" << std::endl;
    return false;
  }

  std::cout << "Set DRP max frequency to " << (1260 / (drp_clock_divider + 1)) << " MHz" << std::endl;

  if ((ai_mac_clock_divider >= 2) && (ai_mac_clock_divider <= 16)) {
    if (ioctl(getFd(), DRPAI_SET_DRPAI_FREQ, &ai_mac_clock_divider) != 0) {
      std::cerr << "[ERROR] Failed to set AI-MAC clock divider" << std::endl;
    }
  } else {
    std::cerr << "[ERROR] Invalid AI-MAC clock divider" << std::endl;
    return false;
  }

  std::cout << "Set AI-MAC frequency to " << (1260 / (ai_mac_clock_divider - 1)) << " MHz" << std::endl;

  return true;
}

/**
 *
 */
bool DrpAiAccelerator::runInference(uint64_t data_in) {
  drpai_data_t proc[DRPAI_INDEX_NUM];
  drpai_adrconv_t addr_info;

  addr_info.org_address = model_base_address_;
  addr_info.size = object_files_size_;
  addr_info.conv_address = drpai_address_map_.start_address;
  addr_info.mode = DRPAI_ADRCONV_MODE_REPLACE;
  if (ioctl(getFd(), DRPAI_SET_ADRCONV, &addr_info) != 0) {
    std::cerr << "[ERROR] Failed to run SET_ADRCONV" << std::endl;
    return false;
  }

  // Changes the input image address to be input to the DRP-AI
  addr_info.org_address = IMG_AREA_ORG_ADDRESS;
  addr_info.size = IMG_AREA_SIZE;
  addr_info.conv_address = IMG_AREA_CNV_ADDRESS;
  addr_info.mode = DRPAI_ADRCONV_MODE_ADD;
  if (ioctl(getFd(), DRPAI_SET_ADRCONV, &addr_info) != 0) {
    std::cerr << "[ERROR] Failed to run SET_ADRCONV for DRP-AI input image" << std::endl;
    return false;
  }

  // Set DRP-AI Driver Input (DRP-AI Object files address and size)
  proc[DRPAI_INDEX_INPUT].address = data_in;
  proc[DRPAI_INDEX_INPUT].size = drpai_address_map_.data_in_size;
  proc[DRPAI_INDEX_DRP_CFG].address = objects_[ObjectType::kDrpConfig].address;
  proc[DRPAI_INDEX_DRP_CFG].size = objects_[ObjectType::kDrpConfig].size;
  proc[DRPAI_INDEX_DRP_PARAM].address = objects_[ObjectType::kDrpParam].address;
  proc[DRPAI_INDEX_DRP_PARAM].size = objects_[ObjectType::kDrpParam].size;
  proc[DRPAI_INDEX_AIMAC_DESC].address = objects_[ObjectType::kAiMacDesc].address;
  proc[DRPAI_INDEX_AIMAC_DESC].size = objects_[ObjectType::kAiMacDesc].size;
  proc[DRPAI_INDEX_DRP_DESC].address = objects_[ObjectType::kDrpDesc].address;
  proc[DRPAI_INDEX_DRP_DESC].size = objects_[ObjectType::kDrpDesc].size;
  proc[DRPAI_INDEX_WEIGHT].address = objects_[ObjectType::kWeight].address;
  proc[DRPAI_INDEX_WEIGHT].size = objects_[ObjectType::kWeight].size;
  proc[DRPAI_INDEX_OUTPUT].address = drpai_address_map_.data_out_addr;
  proc[DRPAI_INDEX_OUTPUT].size = drpai_address_map_.data_out_size;
  proc[DRPAI_INDEX_AIMAC_CMD].address = objects_[ObjectType::kAiMacCmd].address;
  proc[DRPAI_INDEX_AIMAC_CMD].size = objects_[ObjectType::kAiMacCmd].size;
  proc[DRPAI_INDEX_AIMAC_PARAM_DESC].address = objects_[ObjectType::kAiMacParamDesc].address;
  proc[DRPAI_INDEX_AIMAC_PARAM_DESC].size = objects_[ObjectType::kAiMacParamDesc].size;
  proc[DRPAI_INDEX_AIMAC_PARAM_CMD].address = objects_[ObjectType::kAiMacParamCmd].address;
  proc[DRPAI_INDEX_AIMAC_PARAM_CMD].size = objects_[ObjectType::kAiMacParamCmd].size;

  // Start infererence
  if (ioctl(getFd(), DRPAI_START, &proc[0]) != 0) {
    std::cerr << "[ERROR] Failed to run DRPAI_START" << std::endl;
    return false;
  }

  return true;
}

/**
 * @todo Eliminate memcpy() calls by writing directly to the output buffer
 */
bool DrpAiAccelerator::getResult(float *buffer, uint32_t length_bytes) {
  // Set the memory address and size to be read
  drpai_data_t drpai_data;
  drpai_data.address = drpai_address_map_.data_out_addr;
  drpai_data.size = length_bytes;

  if (ioctl(getFd(), DRPAI_ASSIGN, &drpai_data) != 0) {
    std::cerr << "[ERROR] DRPAI_ASSIGN ioctl failed" << std::endl;
    return false;
  }

  // Read from the DRP-AI device and copy the data to the output buffer
  float drpai_buf[BUF_SIZE];

  for (uint64_t i = 0; i < (drpai_data.size / BUF_SIZE); i++) {
    if (read(getFd(), drpai_buf, BUF_SIZE) == -1) {
      std::cerr << "[ERROR] Failed to read from DRP-AI device" << std::endl;
      return false;
    }
    memcpy(&buffer[BUF_SIZE / sizeof(float) * i], drpai_buf, BUF_SIZE);
  }

  if (0 != (drpai_data.size % BUF_SIZE)) {
    if (read(getFd(), drpai_buf, (drpai_data.size % BUF_SIZE)) == -1) {
      std::cerr << "[ERROR] Failed to read from DRP-AI device" << std::endl;
      return false;
    }
    memcpy(&buffer[(drpai_data.size - (drpai_data.size % BUF_SIZE)) / sizeof(float)], drpai_buf,
           (drpai_data.size % BUF_SIZE));
  }

  return true;
}

/*
 * Private
 */

/**
 * @details Format of addr_map.txt is:
 *
 * element address size
 * element address size
 * element address size
 * ...
 *
 * Where element is a string, and address and size are ASCII-formatted hexadecimal numbers
 *
 * @todo Verify that all elements are present in addr_map.txt?
 */
bool DrpAiAccelerator::readAddressMap() {
  std::string addr_file = object_directory_ + "/addr_map.txt";
  std::ifstream ifs(addr_file);

  if (ifs.fail()) {
    std::cerr << "[ERROR] Failed to open address map list : " << addr_file << std::endl;
    return false;
  }

  std::string line;
  uint64_t address = 0;
  uint32_t size = 0;

  // Base address for all objects; will be adjusted when the "data_in" element is read
  uint64_t base_address = drpai_address_map_.start_address;

  while (std::getline(ifs, line)) {
    std::istringstream iss(line);

    std::string element, addr_str, size_str;
    iss >> element >> addr_str >> size_str;

    address = std::stoull(addr_str, nullptr, 16);
    size = std::stoul(size_str, nullptr, 16);

    /**
     * @note "data_in" should appear first in the addr_map.txt file, as its address is used to adjust all the others.
     * It has been moved to the top of this if/else tree to make it easier to find should changes be required.
     */

    if ("data_in" == element) {
      // "Normalise" the base address
      base_address -= address;
      // Save the model's base address, for use in the runInference() function
      model_base_address_ = address;
      drpai_address_map_.data_in_addr = base_address + address;
      drpai_address_map_.data_in_size = size;
    } else if ("drp_config" == element) {
      objects_[ObjectType::kDrpConfig] = {
          .address = base_address + address, .size = size, .filename = "drp_config.mem"};
    } else if ("aimac_desc" == element) {
      objects_[ObjectType::kAiMacDesc] = {
          .address = base_address + address, .size = size, .filename = "aimac_desc.bin"};
    } else if ("drp_desc" == element) {
      objects_[ObjectType::kDrpDesc] = {.address = base_address + address, .size = size, .filename = "drp_desc.bin"};
    } else if ("drp_param" == element) {
      objects_[ObjectType::kDrpParam] = {.address = base_address + address, .size = size, .filename = "drp_param.bin"};
    } else if ("weight" == element) {
      objects_[ObjectType::kWeight] = {.address = base_address + address, .size = size, .filename = "weight.bin"};
    } else if ("data" == element) {
      drpai_address_map_.data_addr = base_address + address;
      drpai_address_map_.data_size = size;
    } else if ("data_out" == element) {
      drpai_address_map_.data_out_addr = base_address + address;
      drpai_address_map_.data_out_size = size;
    } else if ("work" == element) {
      drpai_address_map_.work_addr = base_address + address;
      drpai_address_map_.work_size = size;
    } else if ("aimac_param_cmd" == element) {
      objects_[ObjectType::kAiMacParamCmd] = {
          .address = base_address + address, .size = size, .filename = "aimac_param_cmd.bin"};
    } else if ("aimac_param_desc" == element) {
      objects_[ObjectType::kAiMacParamDesc] = {
          .address = base_address + address, .size = size, .filename = "aimac_param_desc.bin"};
    } else if ("aimac_cmd" == element) {
      objects_[ObjectType::kAiMacCmd] = {.address = base_address + address, .size = size, .filename = "aimac_cmd.bin"};
    } else {
      // Ignore other lines
    }
  }

  // Use the last entry to find the total size of the objects, as not all the entries are contiguous
  object_files_size_ = address + size - model_base_address_;
  return true;
}

/**
 *
 */
bool DrpAiAccelerator::loadParameterInfo() {
  std::string drpai_param_filename = object_directory_ + "/drp_param_info.txt";
  std::ifstream param_file(drpai_param_filename, std::ifstream::ate);
  uint32_t drp_param_info_size = static_cast<uint32_t>(param_file.tellg());

  drpai_assign_param_t drpai_param;
  drpai_param.info_size = drp_param_info_size;
  drpai_param.obj.address = objects_[ObjectType::kDrpParam].address;
  drpai_param.obj.size = objects_[ObjectType::kDrpParam].size;

  if (ioctl(getFd(), DRPAI_ASSIGN_PARAM, &drpai_param) != 0) {
    std::cerr << "[ERROR] DRPAI_ASSIGN_PARAM ioctl failed" << std::endl;
    return false;
  }

  param_file.seekg(0);

  char buffer[BUF_SIZE];

  while (!param_file.eof()) {
    param_file.read(buffer, BUF_SIZE);
    if (write(getFd(), buffer, param_file.gcount()) < 0) {
      std::cerr << "[ERROR] Failed to write parameter info" << std::endl;
    }
  }

  return true;
}

/**
 *
 */
bool DrpAiAccelerator::loadDataObjects() {
  for (const auto& kv : objects_) {
    if (!loadObjectIntoMemory(kv.second)) {
      std::cerr << "[ERROR] Failed to load data into memory (" << kv.second.filename << ")" << std::endl;
      return false;
    }
  }
  return true;
}

/**
 *
 */
bool DrpAiAccelerator::loadObjectIntoMemory(const DrpAiObject& object) {
  const auto filename = object_directory_ + "/" + object.filename;
  std::cout << "Loading : " << filename << std::endl;

  std::ifstream object_file(filename, std::ifstream::binary);
  if (!object_file.is_open()) {
    std::cerr << "[ERROR] Failed to load " << filename << std::endl;
    return false;
  }

  drpai_data_t drpai_data;
  drpai_data.address = object.address;
  drpai_data.size = object.size;

  if (ioctl(getFd(), DRPAI_ASSIGN, &drpai_data) != 0) {
    std::cerr << "[ERROR] DRPAI_ASSIGN ioctl failed" << std::endl;
    return false;
  }

  char buffer[BUF_SIZE];

  while (!object_file.eof()) {
    object_file.read(buffer, BUF_SIZE);
    if (write(getFd(), buffer, object_file.gcount()) < 0) {
      std::cerr << "[ERROR] Failed to write object file" << std::endl;
    }
  }

  return true;
}
