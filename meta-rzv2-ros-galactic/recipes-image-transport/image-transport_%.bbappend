FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"

FILES:${PN} += "\
  /usr/lib/lib*.so* \
  /usr/lib/image_transport/* \
"
SYSROOT_DIRS:append = " \
  /usr/lib \
  /usr/lib/image_transport \
"
