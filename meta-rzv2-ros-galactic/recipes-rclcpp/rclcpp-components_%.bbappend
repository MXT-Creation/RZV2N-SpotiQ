FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"

FILES:${PN} += "\
  /usr/lib/lib*.so* \
  /usr/lib/rclcpp_components/* \
"
SYSROOT_DIRS:append = " \
  /usr/lib \
  /usr/lib/rclcpp_components \
"

