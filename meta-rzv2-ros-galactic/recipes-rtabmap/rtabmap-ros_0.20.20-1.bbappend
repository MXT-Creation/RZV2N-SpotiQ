
# Apply patches
FILESEXTRAPATHS:prepend := "${THISDIR}/${BPN}:"
SRC_URI += " \
    file://0001-CMakefile-modified.patch \
    "

FILES:${PN} += "\
  /usr/lib/lib*.so* \
  /usr/lib/rtabmap_ros/* \
"

SYSROOT_DIRS:append = " \
  /usr/lib \
  /usr/lib/rtabmap_ros \
"
