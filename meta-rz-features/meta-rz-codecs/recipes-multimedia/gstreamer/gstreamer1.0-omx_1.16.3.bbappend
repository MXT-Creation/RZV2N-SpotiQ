FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}/:"

SRC_URI_remove = "https://gstreamer.freedesktop.org/src/gst-omx/gst-omx-${PV}.tar.xz"

SRC_URI_append = " \
    git://github.com/renesas-rcar/gst-omx.git;branch=RCAR-GEN3e/1.16.3;name=base \
    git://gitlab.freedesktop.org/gstreamer/common;destsuffix=git/common;name=common \
    file://0001-omxvideodec-don-t-drop-frame-if-it-contains-header-d.patch \
    file://0001-Support-Bypass-mode.patch \
    file://0002-Fix-error-Resolution-do-not-match-in-running-case-fi.patch \
    file://0003-Add-lossy-compress-option-and-bypass-property.patch \
    file://0004-gst-pipeline-cannot-corectly-decode-with-vspmfilter-.patch \
    file://0005-meson-add-target_device-option-for-special-define-su.patch \
    file://0006-meson.build-Update-meson-build-configuration-for-RZV.patch \
    file://0006-Set-output-framerate-same-as-input-framerate.patch \
    file://0007-Fix-error-Adaptive-playback-ignores-user-settings.patch \
    file://0007-gstomx-Support-Renesas-HEVC-Extension.patch                \
    file://0008-Support-updating-stride-and-sliceheight-using-input-.patch \
    file://0008-omx-gstomxh264utils-Update-the-supported-profile-and.patch \
    file://0009-omxvideo-add-utilize-function-to-calc-power-value.patch    \
    file://0010-omx-videoenc-Correct-nStride-and-data-layout-for-RZV.patch \
    file://0011-omx-videoenc-Convert-I420-to-NV12-process-for-RZV2H.patch  \
    file://0012-videodec-Update-decode-outport-defination-for-RZ-V2H.patch \
    file://0013-omx-videodec-Remove-configure-bypass-option-for-RZV2.patch \
    file://0014-omxh265enc-handle-CODECCONFIG-buffers-Exact-same-cod.patch \
    file://0015-videodec-Re-factor-calculate-number-of-output-buffer.patch \
    file://0016-videoenc-Update-data-layout-for-input-data.patch           \
    file://0017-h265utils-Update-the-supported-profile-and-level-for.patch \
    file://0018-omxh265enc-Update-option-periodicity-idr.patch             \
    file://0019-omxh265enc-Correct-option-interval_intraframes-flow.patch  \
    file://0020-Set-output-framerate-same-as-input-frame.patch             \
    file://gstomx-rzv2h.conf \
"

require include/rz-path-common.inc

DEPENDS += "codec-user-module mmngrbuf-user-module drpb"

SRCREV_base = "6db86e9434815d27de853b4c8235d098da5500a2"
SRCREV_common = "52adcdb89a9eb527df38c569539d95c1c7aeda6e"


LIC_FILES_CHKSUM = " \
    file://COPYING;md5=4fbd65380cdd255951079008b364516c \
    file://omx/gstomx.h;beginline=1;endline=22;md5=4b2e62aace379166f9181a8571a14882 \
"

S = "${WORKDIR}/git"

GSTREAMER_1_0_OMX_TARGET = "rcar"
GSTREAMER_1_0_OMX_CORE_NAME = "${libdir}/libomxr_core.so"
EXTRA_OEMESON_append = " -Dheader_path=${STAGING_DIR_TARGET}/usr/local/include -Dwith_lossy_compress=false -Dtarget_device=rzv2h "

GSTOMX_CONF = "gstomx-rzv2h.conf"

do_configure_prepend() {
    cd ${S}
    install -m 0644 ${WORKDIR}/${GSTOMX_CONF} ${S}/config/rcar/gstomx.conf
    sed -i 's,@RENESAS_DATADIR@,${RENESAS_DATADIR},g' ${S}/config/rcar/gstomx.conf
    ./autogen.sh --noconfigure
    cd ${B}
}

RDEPENDS_${PN}_append = " codec-user-module"
RDEPENDS_${PN}_remove = "libomxil"
