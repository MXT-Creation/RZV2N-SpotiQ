FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"

FILES:${PN} += "\
  /usr/lib/lib*.so* \
  /usr/lib/nav2_map_server/* \
"
SYSROOT_DIRS:append = " \
  /usr/lib \
  /usr/lib/nav2_map_server \
"

