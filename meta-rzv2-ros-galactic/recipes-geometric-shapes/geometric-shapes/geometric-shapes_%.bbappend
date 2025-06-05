FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"

FILES:${PN} += "\
  /usr/lib/lib*.so* \
"
FILES:${PN}-dev += "\
  /usr/lib/lib*.so* \
"
SYSROOT_DIRS:append = " \
  /usr/lib \
"

