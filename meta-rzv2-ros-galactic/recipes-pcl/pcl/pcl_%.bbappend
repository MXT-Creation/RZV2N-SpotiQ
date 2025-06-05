FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"

FILES:${PN} += "\
  /usr/lib/lib*.so* \
  /usr/lib/pkgconfig/* \
"
SYSROOT_DIRS:append = " \
  /usr/lib \
  /usr/pkgconfig \
"
inherit ros_insane_dev_so
INSANE_SKIP_${PN} += " build-deps"
