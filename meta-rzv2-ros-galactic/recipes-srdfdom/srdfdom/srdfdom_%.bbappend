FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"

FILES:${PN} += "\
  /usr/lib/lib*.so* \
  /usr/lib/srdfdom/* \
  /usr/lib/srdfdom/display_srdf/* \
"

SYSROOT_DIRS:append = " \
  /usr/lib \
  /usr/lib/srdfdom \
  /usr/lib/srdfdom/display_srdf \
"

