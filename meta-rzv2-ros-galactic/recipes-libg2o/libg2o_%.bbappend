FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"
SRC_URI += " \
    file://0001-disable-opengl.patch \
    "

FILES:${PN} += "\
  /usr/lib/lib*.so* \
  /usr/lib/cmake/g2o/* \
"
SYSROOT_DIRS:append = " \
  /usr/lib \
  /usr/lib/cmake \
  /usr/lib/cmake/g2o \
"

DEPENDS_remove = "libglu mesa"
RDEPENDS:${PN}:remove = "libglu mesa"

EXTRA_OECMAKE += " -DG2O_USE_OPENGL=OFF"

