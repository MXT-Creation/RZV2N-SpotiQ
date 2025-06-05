FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"

FILES:${PN} += "\
  /usr/lib/lib*.so* \
"

FILES_${PN}-staticdev += "\
  /usr/lib/lib*.a \
"

SYSROOT_DIRS:append = " \
  /usr/lib \
"

