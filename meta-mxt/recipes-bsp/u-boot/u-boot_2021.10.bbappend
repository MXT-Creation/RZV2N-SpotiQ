FILESEXTRAPATHS_prepend := "${THISDIR}/files:"

SRC_URI_append = " \
	file://patches/0001-set-reset-high-for-usb-eth.patch \
"
