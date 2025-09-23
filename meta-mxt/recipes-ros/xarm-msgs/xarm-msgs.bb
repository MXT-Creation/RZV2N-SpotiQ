SUMMARY = "xArm C++ SDK library (ament package)"
AUTHOR = "Marius Muresan <marius.muresan@mxt.ro>"
LICENSE = "BSD-3-Clause"
LIC_FILES_CHKSUM = "file://../LICENSE;md5=55e29519914eb9bed7ac296f34c330f5"

SRC_URI = "gitsm://github.com/xArm-Developer/xarm_ros2.git;branch=humble;protocol=https"
SRCREV = "${AUTOREV}"


# Build only the subpackage directory
S = "${WORKDIR}/git/xarm_msgs"
ROS_PACKAGE_NAME = "xarm_msgs"

inherit ros_ament_cmake ros_distro_humble

# Add any runtime deps here if you later discover they’re needed.
DEPENDS += "\
    rclcpp \
    std-msgs \
    geometry-msgs \
    rosidl-default-generators \
    rosidl-adapter-native \
    ament-cmake-ros \
    ament-cmake-gmock \
    ament-cmake-gtest \
    ament-cmake-pytest \
    rosidl-default-generators-native \
    rosidl-adapter-native \
    rosidl-parser-native \
    python3-empy-native \
"

RDEPENDS:${PN} += "rosidl-default-runtime"

# Export CMake package dirs to dependent recipes' sysroots
SYSROOT_DIRS:append = " \
  ${ros_prefix}/share/xarm_msgs/cmake \
  ${ros_prefix}/lib/cmake/xarm_msgs \
"

# Packaging
FILES:${PN} += "${ros_prefix}/*"
