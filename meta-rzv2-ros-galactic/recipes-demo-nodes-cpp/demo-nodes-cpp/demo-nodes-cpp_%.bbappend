FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"

FILES:${PN} += "\
  /usr/lib/lib*.so* \
  /usr/lib/demo_nodes_cpp/* \
"
SYSROOT_DIRS:append = " \
  /usr/lib \
  /usr/lib/demo_nodes_cpp/ \
"

