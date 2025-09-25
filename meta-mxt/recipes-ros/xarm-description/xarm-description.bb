SUMMARY = "xArm api library (ament package)"
AUTHOR = "Marius Muresan <marius.muresan@mxt.ro>"
LICENSE = "BSD-3-Clause"
LIC_FILES_CHKSUM = "file://../LICENSE;md5=55e29519914eb9bed7ac296f34c330f5"

SRC_URI = "gitsm://github.com/xArm-Developer/xarm_ros2.git;branch=humble;protocol=https"
SRCREV = "${AUTOREV}"


# Build only the subpackage directory
S = "${WORKDIR}/git/xarm_description"
ROS_PACKAGE_NAME = "xarm_description"

# Inherit ROS 2 ament-cmake helpers for Humble
inherit ros_ament_cmake ros_distro_humble

# Add any runtime deps here if you later discover they’re needed.
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
"

RDEPENDS:${PN} += ""

# Packaging
FILES:${PN} += "${ros_prefix}/*"
