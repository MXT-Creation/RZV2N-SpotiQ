SUMMARY = "xArm api library (ament package)"
AUTHOR = "Marius Muresan <marius.muresan@mxt.ro>"
LICENSE = "BSD-3-Clause"
LIC_FILES_CHKSUM = "file://../LICENSE;md5=55e29519914eb9bed7ac296f34c330f5"

SRC_URI = " \
	gitsm://github.com/xArm-Developer/xarm_ros2.git;branch=humble;protocol=https \
	file://0001-Enable-gripper.patch \
"

SRCREV = "${AUTOREV}"


# Build only the subpackage directory
S = "${WORKDIR}/git/xarm_api"
ROS_PACKAGE_NAME = "xarm_api"

# Inherit ROS 2 ament-cmake helpers for Humble
inherit ros_ament_cmake ros_distro_humble

# Add any runtime deps here if you later discover they’re needed.
DEPENDS += "\
    xarm-msgs \
    xarm-sdk \
    rclcpp \
    rclcpp-action \
    action-msgs \
    sensor-msgs \
    std-msgs \
    geometry-msgs \
    control-msgs \
    trajectory-msgs \
    std-srvs \
    ament-cmake-native \
    rosidl-adapter \
"
# Make find_package() deterministic; include both share/ and lib/cmake/ variants
EXTRA_OECMAKE:append = " \
  -Dxarm_msgs_DIR=${RECIPE_SYSROOT}${ros_prefix}/share/xarm_msgs/cmake \
  -Dxarm_msgs_DIR2=${RECIPE_SYSROOT}${ros_prefix}/lib/cmake/xarm_msgs \
  -Dxarm_sdk_DIR=${RECIPE_SYSROOT}${ros_prefix}/share/xarm_sdk/cmake \
  -Dxarm_sdk_DIR2=${RECIPE_SYSROOT}${ros_prefix}/lib/cmake/xarm_sdk \
  -DCMAKE_PREFIX_PATH=${RECIPE_SYSROOT}${ros_prefix} \
"

RDEPENDS:${PN} += "xarm-msgs xarm-sdk"

# Packaging
FILES:${PN} += "${ros_prefix}/*"
