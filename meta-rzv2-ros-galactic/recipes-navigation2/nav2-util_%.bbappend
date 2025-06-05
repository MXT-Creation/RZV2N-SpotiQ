FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"

FILES:${PN} += "\
  /usr/lib/lib*.so* \
  /usr/lib/nav2_util/* \
"
SYSROOT_DIRS:append = " \
  /usr/lib \
  /usr/lib/nav2_util/ \
"


