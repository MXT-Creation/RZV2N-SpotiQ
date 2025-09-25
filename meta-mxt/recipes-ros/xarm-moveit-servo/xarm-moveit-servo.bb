SUMMARY = "xArm MoveIt Servo nodes (ROS 2 Humble)"
AUTHOR = "Marius Muresan <marius.muresan@mxt.ro>"
LICENSE = "BSD-3-Clause"
LIC_FILES_CHKSUM = "file://../LICENSE;md5=55e29519914eb9bed7ac296f34c330f5"

# Example
FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

SRC_URI = "\
    git://github.com/xArm-Developer/xarm_ros2.git;branch=humble;protocol=https \
    file://001_robot_moveit_servo_realmove.patch;striplevel=2  \
"

SRCREV = "${AUTOREV}"

# Build only the xarm_moveit_servo package
S = "${WORKDIR}/git/xarm_moveit_servo"
ROS_PACKAGE_NAME = "xarm_moveit_servo"

# Inherit ROS 2 ament-cmake helpers for Humble
inherit ros_ament_cmake ros_distro_humble

# ament bits needed during configure
DEPENDS += "\
  ament-cmake \
  ament-cmake-libraries \
  ament-package-native \
  ament-cmake-export-definitions \
  ament-cmake-export-include-directories \
  ament-cmake-export-interfaces \
  ament-cmake-export-libraries \
  ament-cmake-export-link-flags \
  ament-cmake-export-targets \
  ament-cmake-gen-version-h \
  ament-cmake-python \
  ament-cmake-target-dependencies \
  ament-cmake-include-directories \
  ament-cmake-test \
  ament-cmake-version \
  python3-catkin-pkg-native \
  rclcpp \
  rclcpp-components \
  std-msgs \
  std-srvs \
  sensor-msgs \
  control-msgs \
  moveit-msgs \
  geometry-msgs \
  moveit-ros-planning \
"
RDEPENDS:${PN} += ""

# Packaging
FILES:${PN} += "${ros_prefix}/*"

