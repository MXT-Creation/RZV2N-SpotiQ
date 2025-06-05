FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"

FILES:${PN} += "\
  /usr/lib/lib*.so* \
"
SYSROOT_DIRS:append = " \
  /usr/lib \
"

# https://github.com/ros/meta-ros/issues/1021
ROS_BUILD_DEPENDS += "builtin-interfaces"
ROS_BUILDTOOL_DEPENDS += "rosidl-typesupport-fastrtps-cpp-native"
ROS_BUILDTOOL_DEPENDS += "rosidl-typesupport-fastrtps-c-native"

