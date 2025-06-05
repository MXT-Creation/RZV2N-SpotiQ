FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"

FILES:${PN} += "\
  /usr/lib/lib*.so* \
  /usr/lib/theora_image_transport/lib*.so* \
"
SYSROOT_DIRS:append = " \
  /usr/lib/theora_image_transport \
  /usr/lib \
"
