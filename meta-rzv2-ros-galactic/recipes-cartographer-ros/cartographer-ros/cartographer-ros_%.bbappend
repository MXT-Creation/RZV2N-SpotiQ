FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"

FILES:${PN} += "\
  /usr/lib/lib*.so* \
  /usr/lib/lib*.a \
  /usr/lib/cartographer_ros/* \
"
FILES:${PN}-staticdev += "\
  /usr/lib/lib*.a \
"
SYSROOT_DIRS:append = " \
  /usr/lib \
  /usr/lib/cartographer_ros \
"

