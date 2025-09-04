DESCRIPTION = "Trusted Firmware-A for Renesas RZ"

require recipes-bsp/trusted-firmware-a/trusted-firmware-a.inc

DEPENDS:append = " openssl"
EXTRA_OEMAKE:append ?= " BOARD=${TFA_BOARD}"

FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

SRC_URI:append = " \
	file://0001-dmd-on-setting.patch \
"
