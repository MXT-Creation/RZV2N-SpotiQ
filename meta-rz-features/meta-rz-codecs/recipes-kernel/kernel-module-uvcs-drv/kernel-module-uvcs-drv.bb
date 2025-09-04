DESCRIPTION = "Kernel module of UVCS"
LICENSE = "GPL-2.0-only & MIT"
LIC_FILES_CHKSUM = " \
    file://include/GPL-COPYING;md5=b234ee4d69f5fce4486a80fdaf4a4263 \
    file://src/lkm/GPL-COPYING;md5=b234ee4d69f5fce4486a80fdaf4a4263 \
    file://src/core/GPL-COPYING;md5=b234ee4d69f5fce4486a80fdaf4a4263 \
    file://src/cmn/GPL-COPYING;md5=b234ee4d69f5fce4486a80fdaf4a4263 \
    file://include/MIT-COPYING;md5=fea016ce2bdf2ec10080f69e9381d378 \
    file://src/lkm/MIT-COPYING;md5=fea016ce2bdf2ec10080f69e9381d378 \
    file://src/core/MIT-COPYING;md5=fea016ce2bdf2ec10080f69e9381d378 \
    file://src/cmn/MIT-COPYING;md5=fea016ce2bdf2ec10080f69e9381d378 \
"
inherit module
require include/rz-modules-common.inc
PR = "r0"

COMPATIBLE_MACHINE = "rzv2h-family"
SRC_URI = "${@oe.utils.conditional('USE_VIDEO_OMX', '1', 'file://uvcs_kernel_package_v4.3.3.0.tar.bz2', '', d)}"

S = "${WORKDIR}/uvcs_kernel_package"

EXTRA_OEMAKE = "KERNELDIR=${STAGING_KERNEL_BUILDDIR}"
EXTRA_OEMAKE += "CROSS_COMPILE=${CROSS_COMPILE}"

B="${S}/src/makefile"

includedir = "${RENESAS_DATADIR}/include"

do_fetch[file-checksums] = ""

do_compile:prepend() {
    export UVCS_SRC="${S}/src"
    export UVCS_INC="${S}"
    export VCP4_SRC="${S}/src"
}

# Build UVCS kernel module without suffix
KERNEL_MODULE_PACKAGE_SUFFIX = ""

do_install() {
    # Create destination directory
    install -d ${D}/${nonarch_base_libdir}/modules/${KERNEL_VERSION}/extra/
    install -d ${D}/${includedir}/

    # Install kernel module
    install -m 644 ${B}/uvcs_drv.ko ${D}/${nonarch_base_libdir}/modules/${KERNEL_VERSION}/extra/

    # Install module symbol file
    install -m 644 ${B}/Module.symvers ${STAGING_KERNEL_BUILDDIR}/UVCS.symvers

    # Install shared header file
    install -m 644 ${S}/include/uvcs_ioctl.h ${D}/${includedir}/
    install -m 644 ${S}/include/uvcs_cmn.h ${D}/${includedir}/
    install -m 644 ${S}/include/uvcs_types.h ${D}/${includedir}/
    install -m 644 ${S}/include/mcvx_api.h ${D}/${includedir}/
    install -m 644 ${S}/include/mcvx_types.h ${D}/${includedir}/
    install -m 644 ${S}/src/cmn/uvcs_cmn_internal.h ${D}/${includedir}/
    install -m 644 ${S}/src/core/mcvx_arch_belz.h ${D}/${includedir}/
    install -m 644 ${S}/src/core/mcvx_arch_fcpc.h ${D}/${includedir}/
    install -m 644 ${S}/src/core/mcvx_inner.h ${D}/${includedir}/
    install -m 644 ${S}/src/core/mcvx_register.h ${D}/${includedir}/
}

# Clean up the module symbol file
CLEANFUNCS = "module_clean_symbol"

module_clean_symbol() {
    rm -f ${STAGING_KERNEL_BUILDDIR}/UVCS.symvers
}

PACKAGES = " \
    ${PN} \
    ${PN}-sstate \
    ${PN}-dev \
    ${PN}-dbg \
"

FILES:${PN}-sstate = " \
    ${includedir}/uvcs_ioctl.h \
"

FILES:${PN}-dev = " \
    ${includedir}/uvcs_cmn.h \
    ${includedir}/uvcs_cmn_internal.h \
    ${includedir}/uvcs_types.h \
    ${includedir}/mcvx_api.h \
    ${includedir}/mcvx_types.h \
    ${includedir}/mcvx_arch_belz.h \
    ${includedir}/mcvx_arch_fcpc.h \
    ${includedir}/mcvx_inner.h \
    ${includedir}/mcvx_register.h \
"

KERNEL_MODULE_AUTOLOAD = "uvcs_drv"
KERNEL_MODULE_PROBECONF = "uvcs_drv"
module_conf_uvcs_drv = "options uvcs_drv ip_option=0x3000A"

