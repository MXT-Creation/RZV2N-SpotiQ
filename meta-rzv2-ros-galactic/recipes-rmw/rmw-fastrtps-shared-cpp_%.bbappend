FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"

FILES:${PN} += "\
  /usr/lib/lib*.so* \
"
SYSROOT_DIRS:append = " \
  /usr/lib \
"
ROS_BUILD_DEPENDS += " \
  rosidl-adapter \
  rmw-dds-common \
"
