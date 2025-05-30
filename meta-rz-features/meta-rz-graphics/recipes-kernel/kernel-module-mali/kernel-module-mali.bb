SUMMARY = "A Mali GPU Linux Kernel module"
SECTION = "kernel/modules"
LICENSE = "GPLv2"

LIC_FILES_CHKSUM = " \
        file://license.txt;md5=13e14ae1bd7ad5bff731bba4a31bb510 "

PR = "p0"
PN = "kernel-module-mali"

inherit module
require include/rz-modules-common.inc
require include/mali-package.inc

SRC_URI = " \
	file://mali-g31_km_${MALI_VERSION}.tar.gz \
	file://0001-Add-reset-control-for-driver.patch \
	file://0001-support-for-linux-kernel-v5.10.patch \
	file://0002-Set-the-performance-governor-by-default.patch \
	file://0003-fix-kernel-warning-when-try-to-enable-disable-regula.patch \
	file://0004-Fix-kernel-warning-of-irq-names.patch \
"

S = "${WORKDIR}/mali_km"

COMPATIBLE_MACHINE = "(r9a07g044l|r9a07g054l|r9a07g044c|r9a09g057|r9a09g056)"

do_fetch[file-checksums] = ""

MALI_KBASE_DIR = "drivers/gpu/arm/midgard"
EXTRA_OEMAKE = 'KDIR="${STAGING_KERNEL_DIR}" \
                ARCH="${ARCH}" \
                BUILD=release \
                CROSS_COMPILE="${CROSS_COMPILE}" \
                MALI_PLATFORM_NAME="devicetree" \
                CONFIG_MALI_MIDGARD=m \
                '
module_do_compile() {
        cd ${MALI_KBASE_DIR}
        oe_runmake
}

module_do_install() {
        install -d ${D}/lib/modules/${KERNEL_VERSION}/extra
        cd ${MALI_KBASE_DIR}
        install -m 644 mali_kbase.ko ${D}/lib/modules/${KERNEL_VERSION}/extra/
        install -m 644 Module.symvers ${STAGING_KERNEL_BUILDDIR}/mali.symvers
}

FILES_${PN} = " \
        /lib/modules/${KERNEL_VERSION}/extra/mali_kbase.ko \
"

PACKAGES = "\
        ${PN} \
"

RPROVIDES_${PN} += "kernel-module-mali"

KERNEL_MODULE_AUTOLOAD_append = " mali_kbase"
