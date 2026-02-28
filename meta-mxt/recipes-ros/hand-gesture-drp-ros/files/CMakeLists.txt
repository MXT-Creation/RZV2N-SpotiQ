cmake_minimum_required(VERSION 3.5)
project(hand_gesture_drp_ros)

set(CMAKE_CXX_STANDARD 17)
add_definitions(-O4 -g)

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(cv_bridge REQUIRED)
find_package(OpenCV REQUIRED)

set(dependencies
  rclcpp
  std_msgs
  sensor_msgs
  cv_bridge
)

add_executable(hand_gesture_recognition_node
  src/hand_gesture_recognition_node.cpp
  src/box.cpp
  drp-ai_tvm_v251/MeraDrpRuntimeWrapper.cpp
  drp-ai_tvm_v251/PreRuntimeV2H.cpp
)

# Include dirs (preferabil target_* în loc de include_directories)
target_include_directories(hand_gesture_recognition_node PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${CMAKE_CURRENT_SOURCE_DIR}/include/hand_gesture_recognition
  ${CMAKE_CURRENT_SOURCE_DIR}/drp-ai_tvm_v251
  ${OpenCV_INCLUDE_DIRS}
)

# TVM headers (dacă TVM_HOME e setat)
set(TVM_ROOT $ENV{TVM_HOME})
if(TVM_ROOT)
  target_include_directories(hand_gesture_recognition_node PRIVATE
    ${TVM_ROOT}/include
    ${TVM_ROOT}/3rdparty/dlpack/include
    ${TVM_ROOT}/3rdparty/dmlc-core/include
    ${TVM_ROOT}/3rdparty/compiler-rt
  )
endif()

ament_target_dependencies(hand_gesture_recognition_node ${dependencies})

# --- TVM runtime lib din sysroot ---
find_library(TVM_RUNTIME_LIB
  NAMES tvm_runtime
  PATHS
    ${CMAKE_SYSROOT}/usr/lib
    ${CMAKE_SYSROOT}/usr/lib64
  NO_DEFAULT_PATH
)

if(NOT TVM_RUNTIME_LIB)
  message(FATAL_ERROR "Could not find libtvm_runtime.so in sysroot (expected in ${CMAKE_SYSROOT}/usr/lib*).")
endif()

target_link_libraries(hand_gesture_recognition_node
  pthread
  mmngr
  mmngrbuf
  ${OpenCV_LIBS}
  ${TVM_RUNTIME_LIB}
)

target_compile_definitions(hand_gesture_recognition_node PRIVATE OPENCV)

install(TARGETS hand_gesture_recognition_node
  RUNTIME DESTINATION lib/${PROJECT_NAME}
)

install(DIRECTORY launch config
  DESTINATION share/${PROJECT_NAME}/
)

ament_package()
