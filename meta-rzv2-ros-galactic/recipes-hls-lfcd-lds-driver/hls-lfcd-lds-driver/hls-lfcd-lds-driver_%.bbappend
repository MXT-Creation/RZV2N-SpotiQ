FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"

FILES:${PN} += "\
  /usr/lib/lib*.so* \
  /usr/lib/hls_lfcd_lds_driver/* \
"
SYSROOT_DIRS:append = " \
  /usr/lib \
  /usr/lib/hls_lfcd_lds_driver \
"

