inherit ros_distro_galactic
inherit ros_superflore_generated

DESCRIPTION = "RTAB-Map's ros-pkg. RTAB-Map is a RGB-D SLAM approach with real-time constraints."
SECTION = "devel"
LICENSE = "BSD"
LIC_FILES_CHKSUM = "file://package.xml;beginline=8;endline=8;md5=88c1aaf10b234b7ed00638bc292d6535"

ROS_CN = "rtabmap_ros"
ROS_BPN = "rtabmap_ros"

ROS_BUILD_DEPENDS = " \
  cv-bridge \
  rclcpp-components \
  rclcpp \
  rclpy \
  sensor-msgs \
  std-msgs \
  std-srvs \
  nav-msgs \
  stereo-msgs \
  geometry-msgs \
  visualization-msgs \
  rosgraph-msgs \
  image-transport \
  compressed-depth-image-transport \
  compressed-image-transport \
  tf2 \
  tf2-eigen \
  tf2-ros \
  tf2-geometry-msgs \
  laser-geometry \
  pcl \
  pcl-conversions \
  message-filters \
  class-loader \
  rtabmap \
  octomap-msgs \
  image-geometry \
  pluginlib \
  nav2-msgs \
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
  cv-bridge \
  rclcpp-components \
  rclcpp \
  rclpy \
  sensor-msgs \
  std-msgs \
  std-srvs \
  nav-msgs \
  stereo-msgs \
  geometry-msgs \
  visualization-msgs \
  rosgraph-msgs \
  image-transport \
  compressed-depth-image-transport \
  compressed-image-transport \
  tf2 \
  tf2-eigen \
  tf2-ros \
  tf2-geometry-msgs \
  laser-geometry \
  pcl \
  pcl-conversions \
  message-filters \
  class-loader \
  rtabmap \
  octomap-msgs \
  image-geometry \
  pluginlib \
  nav2-msgs \
  builtin-interfaces \
"

ROS_BUILDTOOL_EXPORT_DEPENDS = ""

ROS_EXEC_DEPENDS = " \
  cv-bridge \
  rclcpp-components \
  rclcpp \
  rclpy \
  sensor-msgs \
  std-msgs \
  std-srvs \
  nav-msgs \
  stereo-msgs \
  geometry-msgs \
  visualization-msgs \
  rosgraph-msgs \
  image-transport \
  compressed-depth-image-transport \
  compressed-image-transport \
  tf2 \
  tf2-eigen \
  tf2-ros \
  tf2-geometry-msgs \
  laser-geometry \
  pcl \
  pcl-conversions \
  message-filters \
  class-loader \
  rtabmap \
  octomap-msgs \
  image-geometry \
  pluginlib \
  nav2-msgs \
  builtin-interfaces \
  rosidl-default-runtime \
"

# Currently informational only -- see http://www.ros.org/reps/rep-0149.html#dependency-tags.
ROS_TEST_DEPENDS = ""

DEPENDS = "${ROS_BUILD_DEPENDS} ${ROS_BUILDTOOL_DEPENDS}"
# Bitbake doesn't support the "export" concept, so build them as if we needed them to build this package (even though we actually
# don't) so that they're guaranteed to have been staged should this package appear in another's DEPENDS.
DEPENDS += "${ROS_EXPORT_DEPENDS} ${ROS_BUILDTOOL_EXPORT_DEPENDS}"

RDEPENDS:${PN} += "${ROS_EXEC_DEPENDS}"

ROS_BRANCH ?= "branch=release/galactic/rtabmap_ros"
SRC_URI = "git://github.com/introlab/rtabmap_ros-release;${ROS_BRANCH};protocol=https"
SRCREV = "b3398959bc3ac54ba935a21d959ecb3de9cc02fc"
S = "${WORKDIR}/git"

ROS_BUILD_TYPE = "ament_cmake"

inherit ros_${ROS_BUILD_TYPE}
