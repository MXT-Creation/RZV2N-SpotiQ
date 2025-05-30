DESCRIPTION = "OMX Media Components RZG2"
LICENSE = "CLOSED"
require include/rz-modules-common.inc

DEPENDS = " \
    kernel-module-mmngr mmngr-user-module \
    vspmif-user-module kernel-module-vspmif \
    kernel-module-vspm \
"

require codec-package-rzv2.inc

# Task Control. Compile is not performed when not installing OMX Video and Audio Libs.
# Note) dummy-codec-user-module.inc does not exist.
INCLUDE_FILE = '${@oe.utils.conditional("USE_VIDEO_OMX", "1", "dummy", "deltask", d )}'
include ${INCLUDE_FILE}-codec-user-module.inc

DEPENDS += '${@oe.utils.conditional("USE_VIDEO_OMX", "1", "kernel-module-uvcs-drv", "", d )}'

inherit autotools

includedir = "${RENESAS_DATADIR}/include"
CFLAGS += " -I${STAGING_DIR_HOST}${RENESAS_DATADIR}/include"
PACKAGE_ARCH = "${MACHINE_ARCH}"

OMX_EVA_PREFIX = '${@oe.utils.conditional("USE_OMX_EVA_PKG", "1", "EVA", "", d )}'

do_fetch[file-checksums] = ""
do_compile[noexec] = "1"
do_package_qa[noexec] = "1"

# Create ${S} directory
do_unpack_prepend() {
    os.system("install -d ${S}")
}

B = "${S}"

EXTRA_OECONF = "OMXR_DEFAULT_CONFIG_FILE_NAME=${sysconfdir}/omxr/omxr_config_base.txt"

do_install() {
    # Create destination directory
    install -d ${D}/${libdir}
    install -d ${D}/${includedir}
    install -d ${D}/${sysconfdir}/omxr

    if [ "X${USE_VIDEO_OMX}" = "X1" ]; then
        # Copy omx video library
        do_install_omx_video
    fi
}

INSANE_SKIP_${PN} = "dev-so"

FILES_${PN} += " \
    ${libdir}/*.so \
"

FILES_${PN}-dev = " \
    ${includedir} \
    ${libdir}/*.la \
"

RDEPENDS_${PN} += "mmngr-user-module vspmif-user-module"

#To avoid already-stripped errors and not stripped libs from packages
INSANE_SKIP_${PN} += "already-stripped"

# Skip debug split and strip of do_package()
INHIBIT_PACKAGE_DEBUG_SPLIT = "1"
