FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}/:"

SRC_URI:remove = "https://gstreamer.freedesktop.org/src/gst-omx/gst-omx-${PV}.tar.xz"

SRC_URI:append = " \
    git://github.com/renesas-rz/gst-omx.git;branch=RZ/1.22.12;protocol=https \
    file://gstomx.conf \
"

require include/rz-modules-common.inc

DEPENDS += "codec-user-module mmngrbuf-user-module"

SRCREV = "a81110cc5754a9227023f2aa159402fb65597e68"

LIC_FILES_CHKSUM = " \
    file://COPYING;md5=4fbd65380cdd255951079008b364516c \
    file://omx/gstomx.h;beginline=1;endline=22;md5=4b2e62aace379166f9181a8571a14882 \
"

S = "${WORKDIR}/git"

GSTREAMER_1_0_OMX_TARGET = "rz"
GSTREAMER_1_0_OMX_CORE_NAME = "${libdir}/libomxr_core.so"
EXTRA_OEMESON:append = " \
    -Dheader_path=${STAGING_DIR_TARGET}/usr/local/include \
    -Dwith_lossy_compress=false \
"

do_configure:prepend() {
    cd ${S}
    install -m 0644 ${WORKDIR}/gstomx.conf ${S}/config/${GSTREAMER_1_0_OMX_TARGET}/gstomx.conf
    sed -i 's,@RENESAS_DATADIR@,${RENESAS_DATADIR},g' ${S}/config/${GSTREAMER_1_0_OMX_TARGET}/gstomx.conf
    cd ${B}
}

RDEPENDS:${PN}:append = " codec-user-module"
RDEPENDS:${PN}:remove = "libomxil"

