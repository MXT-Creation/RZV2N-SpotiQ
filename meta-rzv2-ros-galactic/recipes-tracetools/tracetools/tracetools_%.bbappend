FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"

FILES:${PN} += "\
  /usr/lib/lib*.so* \
  /usr/lib/tracetools/* \
"
SYSROOT_DIRS:append = " \
  /usr/lib \
  /usr/lib/tracetools \
"

