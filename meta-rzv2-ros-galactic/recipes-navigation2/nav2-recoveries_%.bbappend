FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"

FILES:${PN} += "\
  /usr/lib/lib*.so* \
  /usr/lib/nav2_recoveries/* \
"

FILES_${PN}-staticdev += "\
  /usr/lib/lib*.a \
"

SYSROOT_DIRS:append = " \
  /usr/lib \
  /usr/lib/nav2_recoveries \
"

