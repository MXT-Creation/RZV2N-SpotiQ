FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"

FILES:${PN} += "\
  /usr/lib/lib*.so* \
  /usr/lib/controller_manager/* \
"
SYSROOT_DIRS:append = " \
  /usr/lib \
  /usr/lib/controller_manager \
"

