#LICENSE = "Unknown & MIT"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://LICENSE;md5=54c9df3f89036274f058446106b2f83b"

SRC_URI = "gitsm://github.com/stella-cv/stella_vslam;branch=main \
           file://0001-openmp-flag-add.patch \
"

# Modify these as desired
PV = "1.0+git${SRCPV}"
SRCREV = "86cc9a68c20cbd6a63eafd48391d6bf8c28b410e"

S = "${WORKDIR}/git"

DEPENDS = " sioclient libeigen opencv yaml-cpp libg2o protobuf protobuf-native"

inherit cmake

# Specify any options you want to pass to cmake using EXTRA_OECMAKE:
EXTRA_OECMAKE += " \
    -DUSE_PANGOLIN_VIEWER=OFF \
    -DUSE_SOCKET_PUBLISHER=ON \
    -DINSTALL_SOCKET_PUBLISHER=ON \
    -DUSE_STACK_TRACE_LOGGER=ON \
    -DBUILD_UNIT_TESTS=OFF \
    -DENABLE_PRECOMPILED_HEADERS=OFF \
"

TOOLCHAIN = "clang"

FILES:${PN} += "\
  /usr/lib/lib*.so* \
  /usr/lib/cmake/stella_vslam/* \
"

INSANE_SKIP_${PN} += "dev-so"

SYSROOT_DIRS:append = " \
  /usr/lib \
  /usr/lib/cmake \
  /usr/lib/cmake/stella_vslam \
"
