FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"

FILES:${PN} += "\
  /usr/lib/lib*.so* \
  /usr/lib/realsense2_camera/* \
"
SYSROOT_DIRS:append = " \
  /usr/lib \
  /usr/lib/realsense2_camera/ \
"


