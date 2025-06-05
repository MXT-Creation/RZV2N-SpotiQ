inherit ros_distro_galactic
inherit ros_superflore_generated

DESCRIPTION = "stella_vslam's ros-pkg."
SECTION = "devel"
LICENSE = "BSD"
LIC_FILES_CHKSUM = "file://package.xml;beginline=8;endline=8;md5=553a68b530108aa36ff9ebda853db79b"

ROS_CN = "stella-vslam-ros"
ROS_BPN = "stella-vslam-ros"

ROS_BUILD_DEPENDS = " \
  rclcpp-components \
  rclcpp \
  rclpy \
  libeigen \
  opencv \
  yaml-cpp \
  cv-bridge \
  image-transport \
  message-filters \
  geometry-msgs \
  nav-msgs \
  sensor-msgs \
  tf2 \
  tf2-eigen \
  tf2-geometry-msgs \
  tf2-msgs \
  tf2-ros \
  stella-vslam \
  sioclient \
  protobuf \
  protobuf-native \
  class-loader \
  pluginlib \
  builtin-interfaces \
"

ROS_BUILDTOOL_DEPENDS = " \
    ament-cmake-native \
    rosidl-default-generators-native \
    rosidl-parser-native \
    rosidl-adapter-native \
    rosidl-typesupport-fastrtps-cpp-native \
    rosidl-typesupport-fastrtps-c-native \
    python3-numpy-native \
    python3-lark-parser-native \
"

ROS_EXPORT_DEPENDS = " \
  rclcpp-components \
  rclcpp \
  rclpy \
  libeigen \
  opencv \
  yaml-cpp \
  cv-bridge \
  image-transport \
  message-filters \
  geometry-msgs \
  nav-msgs \
  sensor-msgs \
  tf2 \
  tf2-eigen \
  tf2-geometry-msgs \
  tf2-msgs \
  tf2-ros \
  stella-vslam \
  sioclient \
  protobuf \
  protobuf-native \
  class-loader \
  pluginlib \
  builtin-interfaces \
"

ROS_BUILDTOOL_EXPORT_DEPENDS = ""

ROS_EXEC_DEPENDS = " \
  rclcpp-components \
  rclcpp \
  rclpy \
  libeigen \
  opencv \
  yaml-cpp \
  cv-bridge \
  image-transport \
  message-filters \
  geometry-msgs \
  nav-msgs \
  sensor-msgs \
  tf2 \
  tf2-eigen \
  tf2-geometry-msgs \
  tf2-msgs \
  tf2-ros \
  stella-vslam \
  sioclient \
  protobuf \
  class-loader \
  pluginlib \
  builtin-interfaces \
"

# Currently informational only -- see http://www.ros.org/reps/rep-0149.html#dependency-tags.
ROS_TEST_DEPENDS = ""

DEPENDS = "${ROS_BUILD_DEPENDS} ${ROS_BUILDTOOL_DEPENDS}"
# Bitbake doesn't support the "export" concept, so build them as if we needed them to build this package (even though we actually
# don't) so that they're guaranteed to have been staged should this package appear in another's DEPENDS.
DEPENDS += "${ROS_EXPORT_DEPENDS} ${ROS_BUILDTOOL_EXPORT_DEPENDS}"

RDEPENDS:${PN} += "${ROS_EXEC_DEPENDS}"

ROS_BRANCH ?= "branch=ros2"
#SRC_URI = "git://github.com/stella-cv/stella_vslam_ros;${ROS_BRANCH};protocol=https "
SRC_URI = "gitsm://github.com/stella-cv/stella_vslam_ros;${ROS_BRANCH} \
           file://0001-cmakelists-modify.patch \
"

SRCREV = "7bb959f860ecd3aec35c1805575c3408cdf3dc10"
S = "${WORKDIR}/git"

ROS_BUILD_TYPE = "ament_cmake"

inherit ros_${ROS_BUILD_TYPE}

EXTRA_OECMAKE += " \
    -DUSE_PANGOLIN_VIEWER=OFF \
    -DUSE_SOCKET_PUBLISHER=ON \
    -DINSTALL_SOCKET_PUBLISHER=ON \
    -DUSE_STACK_TRACE_LOGGER=ON \
    -DBUILD_UNIT_TESTS=OFF \
    -DENABLE_PRECOMPILED_HEADERS=OFF \
"

FILES:${PN} += "\
  /usr/lib/stella_vslam_ros/* \
  /usr/share/stella_vslam_ros/* \
  /usr/share/stella_vslam_ros/cmake/* \
  /usr/share/stella_vslam_ros/environment/* \
"

SYSROOT_DIRS:append = " \
  /usr/lib/stella_vslam_ros \
  /usr/share/stella_vslam_ros \
  /usr/share/stella_vslam_ros/cmake \
  /usr/share/stella_vslam_ros/environment \
"
