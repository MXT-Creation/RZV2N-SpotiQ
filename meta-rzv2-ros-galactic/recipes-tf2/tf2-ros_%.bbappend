FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"

FILES:${PN} += "\
  /usr/lib/lib*.so* \
  /usr/lib/tf2_ros/* \
"
SYSROOT_DIRS:append = " \
  /usr/lib \
  /usr/lib/tf2_ros \
"

