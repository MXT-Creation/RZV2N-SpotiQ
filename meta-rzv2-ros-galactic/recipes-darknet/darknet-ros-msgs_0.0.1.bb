inherit ros_distro_galactic
inherit ros_superflore_generated

DESCRIPTION = "darknet ros msgs"
SECTION = "devel"
LICENSE = "BSD"
LIC_FILES_CHKSUM = "file://LICENSE;beginline=8;endline=8;md5=f83e93d0b0d096b0e4b0e027c488b995"

ROS_CN = "darknet_ros_msgs"
ROS_BPN = "darknet_ros_msgs"

ROS_BUILD_DEPENDS = " \
  cv-bridge \
  rclcpp-components \
  rclcpp \
  rclpy \
  sensor-msgs \
  std-msgs \
  image-transport \
  class-loader \
  builtin-interfaces \
  rclcpp-action \
"

ROS_BUILDTOOL_DEPENDS = " \
    ament-cmake-native \
    ament-index-cpp \
    rosidl-default-generators-native \
    rosidl-parser-native \
    rosidl-adapter-native \
    rosidl-typesupport-fastrtps-cpp-native \
    rosidl-typesupport-fastrtps-c-native \
    python3-numpy-native \
    python3-lark-parser-native \
"

ROS_EXPORT_DEPENDS = " \
  cv-bridge \
  rclcpp-components \
  rclcpp \
  rclpy \
  sensor-msgs \
  std-msgs \
  image-transport \
  class-loader \
  builtin-interfaces \
  rclcpp-action \
"

ROS_BUILDTOOL_EXPORT_DEPENDS = ""

ROS_EXEC_DEPENDS = " \
  cv-bridge \
  rclcpp-components \
  rclcpp \
  rclpy \
  sensor-msgs \
  std-msgs \
  image-transport \
  class-loader \
  pluginlib \
  builtin-interfaces \
  rosidl-default-runtime \
  rclcpp-action \
"

# Currently informational only -- see http://www.ros.org/reps/rep-0149.html#dependency-tags.
ROS_TEST_DEPENDS = ""

DEPENDS = "${ROS_BUILD_DEPENDS} ${ROS_BUILDTOOL_DEPENDS}"
# Bitbake doesn't support the "export" concept, so build them as if we needed them to build this package (even though we actually
# don't) so that they're guaranteed to have been staged should this package appear in another's DEPENDS.
DEPENDS += "${ROS_EXPORT_DEPENDS} ${ROS_BUILDTOOL_EXPORT_DEPENDS}"

RDEPENDS:${PN} += "${ROS_EXEC_DEPENDS}"

ROS_BRANCH ?= "branch=foxy"
SRC_URI = "gitsm://github.com/leggedrobotics/darknet_ros;${ROS_BRANCH};protocol=https"
SRCREV = "9e76f3b8fce90a28427b49ad205363431290b887"
S = "${WORKDIR}/git"

ROS_BUILD_TYPE = "ament_cmake"

inherit ros_${ROS_BUILD_TYPE}

FILES:${PN} += "\
  /usr/lib/lib*.so* \
"
SYSROOT_DIRS:append = " \
  /usr/lib \
"

