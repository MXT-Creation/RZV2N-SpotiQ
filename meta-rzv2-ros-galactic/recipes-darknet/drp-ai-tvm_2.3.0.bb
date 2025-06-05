DESCRIPTION = "Extension package of Apache TVM (Machine Learning Compiler) for Renesas DRP-AI accelerators powered by Edgecortix MERA(TM) Based Apache TVM version: v0.11.1"
LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://LICENSE;md5=0ba5044c64ef53cb0189c9546081e228 \
                    file://tvm/LICENSE;md5=6ec4db95cc43836f5e2ff1d6edaa2284 \
                    file://tvm/3rdparty/OpenCL-Headers/LICENSE;md5=3b83ef96387f14655fc854ddc3c6bd57 \
                    file://tvm/3rdparty/cutlass/LICENSE.txt;md5=eee1a08537367b72b6d2fbab971b5878 \
                    file://tvm/3rdparty/dlpack/LICENSE;md5=f62d4e85ba68a1574b74d97ab8dea9ab \
                    file://tvm/3rdparty/dmlc-core/LICENSE;md5=e3fc50a88d0a364313df4b21ef20c29e \
                    file://tvm/3rdparty/libbacktrace/LICENSE;md5=24b5b3feec63c4be0975e1fea5100440 \
                    file://tvm/3rdparty/rang/LICENSE;md5=911690f51af322440237a253d695d19f \
                    file://tvm/3rdparty/vta-hw/LICENSE;md5=e3fc50a88d0a364313df4b21ef20c29e"

RDEPENDS_${PN} += "mmngr-user-module mmngrbuf-user-module"

SRC_URI = "gitsm://github.com/renesas-rz/rzv_drp-ai_tvm;branch=v2.3.0;protocol=https"
SRCREV = "738348bcaa4d584d031019becf67ef2a26e49c67"
S = "${WORKDIR}/git"

PRODUCT="V2H"
do_configure() {
    cd ${S}
    export PRODUCT="${PRODUCT}"
    chmod +x ${S}/setup/make_drp_env.sh
    ./setup/make_drp_env.sh
}

do_install:append() {
    install -d ${D}${libdir} \
               ${D}${includedir}/tvm/runtime \
               ${D}${includedir}/tvm/runtime/container \
               ${D}${includedir}/tvm/3rdparty/dlpack/include/dlpack \
               ${D}${includedir}/tvm/3rdparty/dmlc-core/include/dmlc \
               ${D}${includedir}/tvm/3rdparty/compiler-rt

    install -m 0755 ${S}/tvm/build_runtime/libtvm_runtime.so ${D}${libdir}

    install -m 0644 ${S}/tvm/include/tvm/runtime/*.h ${D}${includedir}/tvm/runtime
    install -m 0644 ${S}/tvm/include/tvm/runtime/container/*.h ${D}${includedir}/tvm/runtime/container
    install -m 0644 ${S}/tvm/3rdparty/dlpack/include/dlpack/*.h ${D}${includedir}/tvm/3rdparty/dlpack/include/dlpack
    install -m 0644 ${S}/tvm/3rdparty/dmlc-core/include/dmlc/*.h ${D}${includedir}/tvm/3rdparty/dmlc-core/include/dmlc
    install -m 0644 ${S}/tvm/3rdparty/compiler-rt/*.h ${D}${includedir}/tvm/3rdparty/compiler-rt

    mkdir -p ${D}${includedir}/linux/
    mkdir -p ${D}${includedir}/uapi/linux/
    install -m 0644 ${STAGING_KERNEL_DIR}/include/linux/drpai.h      ${D}${includedir}/linux/
    install -m 0644 ${STAGING_KERNEL_DIR}/include/uapi/linux/drpai.h ${D}${includedir}/uapi/linux/
    install -m 0644 ${STAGING_KERNEL_DIR}/include/mmngr*.h ${D}${includedir}
    install -m 0644 ${COMPONENTS_DIR}/${MACHINE_ARCH}/mmngr-user-module/usr/local/include/mmngr_user_public.h ${D}${includedir}
    install -m 0644 ${COMPONENTS_DIR}/${MACHINE_ARCH}/mmngrbuf-user-module/usr/local/include/mmngr_buf_user_public.h ${D}${includedir}
    install -m 0755 ${COMPONENTS_DIR}/${MACHINE_ARCH}/mmngr-user-module/usr/lib64/libmmngr.so ${D}${libdir}
    install -m 0755 ${COMPONENTS_DIR}/${MACHINE_ARCH}/mmngrbuf-user-module/usr/lib64/libmmngrbuf.so ${D}${libdir}
}

# Skip already-stripped for QA Check. libmmngr.so and libmmngrbuf.so are already-stripped.
INSANE_SKIP_${PN} += "already-stripped"

FILES_${PN} = "${libdir}/*"
FILES_${PN}-dev = "${includedir}/*"
