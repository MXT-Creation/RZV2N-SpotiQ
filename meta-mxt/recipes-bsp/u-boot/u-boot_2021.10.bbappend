FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

SRC_URI:append = " \
	file://patches/0001-set-reset-high-for-usb-eth.patch \
"

BOOTENV_EXTRA += "usbcore.usbfs_memory_mb=200"
