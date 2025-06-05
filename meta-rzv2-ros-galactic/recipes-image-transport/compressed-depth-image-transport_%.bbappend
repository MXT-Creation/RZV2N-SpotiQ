FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"

FILES:${PN} += "\
  /usr/lib/lib*.so* \
"
SYSROOT_DIRS:append = " \
  /usr/lib \
"
