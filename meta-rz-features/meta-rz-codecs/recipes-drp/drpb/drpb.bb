#
# This recipe adds a shared library and a header file of DRP-B library to RZ/V environment.
#

SUMMARY = "DRPB library"
LICENSE = "CLOSED"

COMPATIBLE_MACHINE = "rzv2h-family"

RDEPENDS:${PN} = " drpb-fw "

# SRC file name
SRC_URI = " \
	file://libdrp_api.so \
	file://drp_api.h \
"

S = "${WORKDIR}"

do_install() {
    install -d ${D}/${libdir}
    install -d ${D}/${includedir}
    install -m 755 ${S}/libdrp_api.so ${D}/${libdir}
    install -m 644 ${S}/drp_api.h ${D}/${includedir}
}

FILES:${PN} += " \
    ${libdir}/libdrp_api.so \
    "

FILES:${PN}-dev = " \
    ${includedir}/drp_api.h \
"

# Skip debug split and strip of do_package()
INHIBIT_PACKAGE_DEBUG_SPLIT = "1"
