DESCRIPTION = "Application get codec information"
LICENSE = "CLOSED"

inherit autotools

do_compile[noexec] = "1"

PV = "1.0.0"

SRC_URI = "file://codecinfo-${PV}.tar.gz"

S = "${WORKDIR}/codecinfo-${PV}"

do_install() {
    # Create destination directory
    install -d ${D}/${bindir}

    # Install application get codec pakage information
    install -m 755 ${S}/get_codec_information/codec-info ${D}/${bindir}
}


FILES:${PN} += " \
    ${bindir}/codec-info \
"

RDEPENDS:${PN}:append = " codec-user-module"

#To avoid already-stripped errors and not stripped libs from packages
INSANE_SKIP:${PN} += " already-stripped"

# Skip debug split and strip of do_package()
INHIBIT_PACKAGE_DEBUG_SPLIT = "1"

