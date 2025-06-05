FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"

FILES:${PN} += "\
  /usr/lib/lib*.so \
  /usr/lib/cmake \
  /usr/lib/cmake/iceoryx_binding_c/* \
"
SYSROOT_DIRS:append = " \
  /usr/lib \
  /usr/lib/cmake \
  /usr/lib/cmake/iceoryx_binding_c \
"

