#
# This recipe adds a shared library and a header file of DRP-B driver to RZ/V2H environment.
#

SUMMARY = "DRPB library"
LICENSE = "CLOSED"

COMPATIBLE_MACHINE = "(r9a09g057|r9a09g056)"

DEPENDS = " codec "
RDEPENDS_${PN} = " codec "

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

FILES_${PN} += " \
    ${libdir}/libdrp_api.so \
    "

FILES_${PN}-dev = " \
    ${includedir}/drp_api.h \
"

# Skip debug split and strip of do_package()
INHIBIT_PACKAGE_DEBUG_SPLIT = "1"
