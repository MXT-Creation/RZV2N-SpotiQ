SUMMARY = "xArm uf-ros-lib"
AUTHOR = "Marius Muresan <marius.muresan@mxt.ro>"
LICENSE = "BSD-3-Clause"
LIC_FILES_CHKSUM = "file://../LICENSE;md5=55e29519914eb9bed7ac296f34c330f5"

SRC_URI = "gitsm://github.com/xArm-Developer/xarm_ros2.git;branch=humble;protocol=https"
SRCREV = "${AUTOREV}"


# Build only the subpackage directory
S = "${WORKDIR}/git/uf_ros_lib"
ROS_PACKAGE_NAME = "uf_ros_lib"

# Inherit ROS 2 ament-cmake helpers for Humble
inherit python_setuptools_build_meta

RDEPENDS:${PN} += "\
  ${PYTHON_PN} \
  ${PYTHON_PN}-pyyaml \
  ${PYTHON_PN}-numpy \
"


PYTHON_SITEPACKAGES_DIR = "${libdir}/python${PYTHON_BASEVERSION}/site-packages"

# Ship both the Python module and the ament index bits
FILES:${PN} += " \
  ${PYTHON_SITEPACKAGES_DIR}/uf_ros_lib* \
  ${datadir}/uf_ros_lib \
  ${datadir}/uf_ros_lib/package.xml \
  ${datadir}/ament_index/resource_index/packages/uf_ros_lib \
"
