FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"

FILES:${PN} += "\
  /usr/lib/lib*.so* \
  /usr/lib/v4l2_camera/* \
"
SYSROOT_DIRS:append = " \
  /usr/lib \
  /usr/lib/v4l2_camera \
"
