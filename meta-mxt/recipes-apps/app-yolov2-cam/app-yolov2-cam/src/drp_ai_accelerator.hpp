/**
 * @file drp_ai_accelerator.hpp
 * @brief
 * @copyright Copyright (c) 2024 IMD Technologies
 */

#include <linux/drpai.h>

#include <map>
#include <memory>

struct DrpAiAddressMap {
  // Set the reserved memory address for the DRP-AI accelerator
  uint64_t start_address;

  // Assigned a value from the address map (0x80000000)
  // Used as a base address when storing the other addresses
  // All addresses end up as an offset from data_in_addr, + start_address
  uint64_t data_in_addr;

  // Assigned a value from the address map (4147200)
  // Equals 1920x1080x2 (16 bits per YUV pixel)
  uint32_t data_in_size;

  // Assigned a value from the address map
  // Never read
  uint64_t data_addr;

  // Assigned a value from the address map
  // Never read
  uint32_t data_size;

  // Assigned a value from the address map
  // Never read
  uint64_t work_addr;

  // Assigned a value from the address map
  // Never read
  uint32_t work_size;

  // Assigned a value from the address map
  // Passed to the DRP-AI device during inference
  uint64_t data_out_addr;

  // Assigned a value from the address map (84500)
  // Equals INF_OUT_SIZE from define.h
  // Passed to the DRP-AI device during inference
  uint32_t data_out_size;
};

enum class ObjectType;
struct DrpAiObject;

class DrpAiAccelerator {
 public:
  using SharedPtr = std::shared_ptr<DrpAiAccelerator>;

  /**
   * @brief Opens the DRP-AI device and reads information about its memory region
   */
  DrpAiAccelerator();

  /**
   * @brief Closes the DRP-AI device
   */
  ~DrpAiAccelerator();

  /**
   * @brief Load the model's object files into memory
   *
   * @param object_directory Path to the object files
   * @return
   */
  bool loadObjectFiles(const std::string& object_directory);

  /**
   * @brief Set the clock dividers for the DRP and AI-MAC
   *
   * @param drp_clock_divider DRP "max frequency" clock divider
   * @param ai_mac_clock_divider AI-MAC clock divider
   * @return
   */
  bool setClockDividers(const uint32_t drp_clock_divider, const uint32_t ai_mac_clock_divider);

  /**
   * @brief Run the inference process
   *
   * @param data_in Image data address
   * @return
   */
  bool runInference(uint64_t data_in);

  /**
   * @brief Get the inference result
   *
   * @param buffer Destination buffer
   * @param length_bytes Number of bytes to read
   * @return
   */
  bool getResult(float *buffer, uint32_t length_bytes);

  /**
   * @brief Get the file descriptor for the DRP-AI device
   *
   * @return File descriptor
   */
  int getFd() const { return fd_; }

 private:
  /**
   * @brief Reads the address map and populates the address info structure
   *
   * @return True if the file was opened
   */
  bool readAddressMap();

  /**
   * @brief Loads the parameter info into memory
   *
   * @return
   */
  bool loadParameterInfo();

  /**
   * @brief Loads various objects into memory
   *
   * @return
   */
  bool loadDataObjects();

  /**
   * @brief Loads a single object into memory
   *
   * @param object The object (filename, address and size)
   * @return
   */
  bool loadObjectIntoMemory(const DrpAiObject& object);

  /// @brief DRP-AI device file descriptor
  int fd_{-1};

  /**
   * @brief Contains the address and size of the memory reserved for the DRP-AI accelerator
   * @details Corresponds to the values under DRP-AI@240000000 in the device tree. Start address is 0x240000000, and
   * size is 0x20000000 (512 MiB).
   */
  drpai_data_t drpai_reserved_memory_;

  /// @brief Address map for the DRP-AI accelerator
  DrpAiAddressMap drpai_address_map_;

  /// @brief Location of the object files for the model
  std::string object_directory_;

  /// @brief Collection of information about the different objects, indexed by the type
  std::map<ObjectType, DrpAiObject> objects_;

  /// @brief Total size of all the object files in the model
  uint32_t object_files_size_;

  // The "data in" address from the model address map
  uint64_t model_base_address_;
};

enum class ObjectType {
  kAiMacCmd,
  kAiMacDesc,
  kAiMacParamCmd,
  kAiMacParamDesc,
  kDrpConfig,
  kDrpDesc,
  kDrpParam,
  kWeight,
};

struct DrpAiObject {
  uint64_t address;
  uint32_t size;
  std::string filename;
};
