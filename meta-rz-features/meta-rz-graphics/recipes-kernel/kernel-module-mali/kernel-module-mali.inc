SUMMARY = "A Mali GPU Linux Kernel module"
SECTION = "kernel/modules"
LICENSE = "GPL-2.0-only"

LIC_FILES_CHKSUM = " \
	file://../../../../license.txt;md5=13e14ae1bd7ad5bff731bba4a31bb510"

PN = "kernel-module-mali"

inherit module
require include/mali-gpu.inc

S = "${WORKDIR}/mali_km/drivers/gpu/arm/midgard"

COMPATIBLE_MACHINE = "(smarc-rzg2l|smarc-rzg2lc|smarc-rzv2l|rzv2n-evk)"

do_fetch[file-checksums] = ""

EXTRA_OEMAKE = 'KDIR="${STAGING_KERNEL_DIR}" \
                ARCH="${ARCH}" \
                BUILD=release \
                CROSS_COMPILE="${CROSS_COMPILE}" \
                MALI_PLATFORM_NAME="devicetree" \
                CONFIG_MALI_MIDGARD=m \
                '

FILES:${PN} = " \
        /lib/modules/${KERNEL_VERSION}/extra/mali_kbase.ko \
"

MODULES_MODULE_SYMVERS_LOCATION = ""

RPROVIDES:${PN} += "kernel-module-mali"

KERNEL_MODULE_AUTOLOAD:append = " mali_kbase"

INSANE_SKIP:${PN}-dbg = "buildpaths"
