#LICENSE = "Unknown & MIT"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://LICENSE;md5=aba3328269a59ed82f8ee7d69431da00 "

SRC_URI = "gitsm://github.com/shinsumicco/socket.io-client-cpp;branch=master \
           file://0001-Fix-cmake-config-to-enable-cmake-on-Windows-to-find-.patch \
           "

# Modify these as desired
PV = "1.0+git${SRCPV}"
SRCREV = "ff6ef08e45c594e33aa6bc19ebdd07954914efe0"

S = "${WORKDIR}/git"

DEPENDS = "openssl"

inherit cmake

# Specify any options you want to pass to cmake using EXTRA_OECMAKE:
EXTRA_OECMAKE = ""

##FILES_${PN} = "${libdir} ${includedir} ${datadir}/cmake/sioclient ${datadir}/pkgconfig"
#FILES_${PN} = "/usr/lib ${includedir} ${datadir}/cmake/sioclient ${datadir}/pkgconfig"
#FILES_${PN}-dev = "/usr/lib ${includedir} ${datadir}/cmake/sioclient ${datadir}/pkgconfig"

FILES:${PN} += "\
  /usr/lib/libsioclient*.so* \
  /usr/lib/cmake/sioclient/* \
  /usr/include/sioclient/* \
"

INSANE_SKIP_${PN} += "dev-so"

SYSROOT_DIRS:append = " \
  /usr/lib \
  /usr/lib/cmake \
  /usr/lib/cmake/sioclient \
  /usr/include/sioclient \
"

BBCLASSEXTEND = " native nativesdk"
