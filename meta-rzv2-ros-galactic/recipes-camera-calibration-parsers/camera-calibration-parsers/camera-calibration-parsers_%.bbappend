FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"

FILES:${PN} += "\
  /usr/lib/lib*.so* \
 /usr/lib/camera_calibration_parsers/* \
"
SYSROOT_DIRS:append = " \
  /usr/lib \
 /usr/lib/camera_calibration_parsers \
"

