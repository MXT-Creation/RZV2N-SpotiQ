FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"

FILES:${PN} += "\
  /usr/lib/lib*.so* \
  /usr/lib/pkgconfig/* \
  /usr/lib/cmake/* \
"

FILES:${PN}-dev += "\
  /usr/lib/lib*.so* \
  /usr/lib/pkgconfig/* \
  /usr/lib/cmake/* \
"

FILES_${PN}-staticdev += "\
  /usr/lib/lib*.a \
"

SYSROOT_DIRS:append = " \
  /usr/lib \
  /usr/lib/pkgconfig \
  /usr/lib/cmake \
"

