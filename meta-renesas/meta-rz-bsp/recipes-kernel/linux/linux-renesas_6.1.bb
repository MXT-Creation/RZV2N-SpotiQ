DESCRIPTION = "Linux kernel from the Renesas RZ BSP based on linux-6.1.y-cip"
FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}/:"

require recipes-kernel/linux/linux-yocto.inc
require linux-renesas_6.1.inc

LINUX_VERSION ?= "6.1.107-cip28"
KBUILD_DEFCONFIG ?= "defconfig"
KCONFIG_MODE ?= "alldefconfig"

KERNEL_URL ?= "git://github.com/renesas-rz/rz_linux-cip.git"
KERNEL_BRANCH = "rzv2n-6.1-cip28"
KERNEL_REV ?= "64919247f56f2512c0884a95bc04250f8aeb46c7"

SRC_URI:append = " \
        file://patches.scc \
"
