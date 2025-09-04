PV="4.3.3.0"
PR="r0"

COMPATIBLE_MACHINE = "rzv2h-family"

OMX_PKG_TAR = "codec_pkg_product_v${PV}.tar.gz"
OMX_PKG = "codec_pkg_product_v${PV}"

RDEPENDS:${PN} = " drpb "

SRC_URI = " \
    file://${OMX_PKG_TAR} \
    file://OMXR_VideoExt.h \
    file://OMXR_IndexExt.h \
"

# SRC directory name
OMX_COMMON_SRC = '${@oe.utils.conditional("USE_VIDEO_OMX", "1", "omx_common", "", d )}'
OMX_VIDEO_DEC_COMMON_SRC = '${@oe.utils.conditional("USE_VIDEO_DEC", "1", "video_dec_common", "", d )}'
OMX_VIDEO_ENC_COMMON_SRC = '${@oe.utils.conditional("USE_VIDEO_ENC", "1", "video_enc_common", "", d )}'
OMX_H264_DEC_SRC = '${@oe.utils.conditional("USE_H264D_OMX", "1", "h264_decoder", "", d )}'
OMX_H264_ENC_SRC = '${@oe.utils.conditional("USE_H264E_OMX", "1", "h264_encoder", "", d )}'
OMX_H265_DEC_SRC = '${@oe.utils.conditional("USE_H265D_OMX", "1", "h265_decoder", "", d )}'
OMX_H265_ENC_SRC = '${@oe.utils.conditional("USE_H265E_OMX", "1", "h265_encoder", "", d )}'

OMX_VIDEO_SRC_LIST = " \
    ${OMX_COMMON_SRC} \
    ${OMX_VIDEO_DEC_COMMON_SRC} \
    ${OMX_VIDEO_ENC_COMMON_SRC} \
    ${OMX_H264_DEC_SRC} \
    ${OMX_H264_ENC_SRC} \
    ${OMX_H265_DEC_SRC} \
    ${OMX_H265_ENC_SRC} \
"
S = "${WORKDIR}/${OMX_PKG}"

do_install_omx_video() {
    cd ${S}
    for omxmc in ${OMX_VIDEO_SRC_LIST}
    do
        install -m 755 ${S}/${omxmc}/lib/lib*.so.* ${D}/${libdir}
        install -m 644 ${S}/${omxmc}/include/*.h ${D}/${includedir}
        install -m 644 ${S}/${omxmc}/config/*.txt ${D}/${sysconfdir}/omxr
    done

    cd ${D}
    install -d ${D}/${includedir}
    install -m 644 ${WORKDIR}/OMXR_VideoExt.h ${D}/${includedir}
    install -m 644 ${WORKDIR}/OMXR_IndexExt.h ${D}/${includedir}

    cd ${D}/${libdir}
    if [ "${USE_VIDEO_OMX}" = "1" ] ; then
        ln -s libomxr_core.so.3.0.0 libomxr_core.so.3
        ln -s libomxr_core.so.3 libomxr_core.so

        ln -s libomxr_mc_cmn.so.3.0.0 libomxr_mc_cmn.so.3
        ln -s libomxr_mc_cmn.so.3 libomxr_mc_cmn.so

        ln -s libomxr_uvcs_udf.so.0.0.0 libomxr_uvcs_udf.so.0
        ln -s libomxr_uvcs_udf.so.0 libomxr_uvcs_udf.so

        ln -s libomxr_utility.so.3.0.0 libomxr_utility.so.3
        ln -s libomxr_utility.so.3 libomxr_utility.so

        ln -s libomxr_mc_vcmn.so.3.0.0 libomxr_mc_vcmn.so.3
        ln -s libomxr_mc_vcmn.so.3 libomxr_mc_vcmn.so
    fi

    if [ "${USE_VIDEO_DEC}" = "1" ] ; then
        ln -s libomxr_mc_vdcmn.so.3.0.0 libomxr_mc_vdcmn.so.3
        ln -s libomxr_mc_vdcmn.so.3 libomxr_mc_vdcmn.so

        ln -s libomxr_videoconverter.so.3.0.0 libomxr_videoconverter.so.3
        ln -s libomxr_videoconverter.so.3 libomxr_videoconverter.so

        ln -s libomxr_cnvpisu.so.0.0.0 libomxr_cnvpisu.so.0
        ln -s libomxr_cnvpisu.so.0 libomxr_cnvpisu.so

        ln -s libomxr_cnvosdep.so.3.0.0 libomxr_cnvosdep.so.3
        ln -s libomxr_cnvosdep.so.3 libomxr_cnvosdep.so

        ln -s libuvcs_dec.so.3.0.0 libuvcs_dec.so.3
        ln -s libuvcs_dec.so.3 libuvcs_dec.so
    fi

    if [ "${USE_VIDEO_ENC}" = "1" ] ; then
        ln -s libomxr_mc_vecmn.so.3.0.0 libomxr_mc_vecmn.so.3
        ln -s libomxr_mc_vecmn.so.3 libomxr_mc_vecmn.so

        ln -s libuvcs_enc.so.3.0.0 libuvcs_enc.so.3
        ln -s libuvcs_enc.so.3 libuvcs_enc.so
    fi

    if [ "${USE_H264D_OMX}" = "1" ]; then
        ln -s libomxr_mc_h264d.so.3.0.0 libomxr_mc_h264d.so.3
        ln -s libomxr_mc_h264d.so.3 libomxr_mc_h264d.so

        ln -s libuvcs_avcd.so.3.0.0 libuvcs_avcd.so.3
        ln -s libuvcs_avcd.so.3 libuvcs_avcd.so
    fi

    if [ "${USE_H264E_OMX}" = "1" ]; then
        ln -s libomxr_mc_h264e.so.3.0.0 libomxr_mc_h264e.so.3
        ln -s libomxr_mc_h264e.so.3 libomxr_mc_h264e.so

        ln -s libuvcs_avce.so.3.0.0 libuvcs_avce.so.3
        ln -s libuvcs_avce.so.3 libuvcs_avce.so
    fi

    if [ "${USE_H265D_OMX}" = "1" ]; then
         ln -s libomxr_mc_h265d.so.3.0.0 libomxr_mc_h265d.so.3
         ln -s libomxr_mc_h265d.so.3 libomxr_mc_h265d.so

         ln -s libuvcs_hevd.so.3.0.0 libuvcs_hevd.so.3
         ln -s libuvcs_hevd.so.3 libuvcs_hevd.so
     fi

     if [ "${USE_H265E_OMX}" = "1" ]; then
         ln -s libomxr_mc_h265e.so.3.0.0 libomxr_mc_h265e.so.3
         ln -s libomxr_mc_h265e.so.3 libomxr_mc_h265e.so

         ln -s libuvcs_heve.so.3.0.0 libuvcs_heve.so.3
         ln -s libuvcs_heve.so.3 libuvcs_heve.so
     fi
}

