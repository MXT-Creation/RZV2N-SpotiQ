#
# Copyright (C) 2025 Renesas Electronics, Co, Ltd.
#
DESCRIPTION = "DRP-AI Driver with kernel for RZ/V2N"

FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}/:"

SRC_URI:append = "\
	file://0001-add-drpai-property-to-devicetree.patch \
	file://0002-enable-drpai-driver.patch \
"
