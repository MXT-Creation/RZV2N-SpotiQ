#
# Copyright (C) 2024 RenesasElectronics, Co, Ltd.
#
DESCRIPTION = "RZ/V2N DRP Driver"

FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}/:"

SRC_URI:append = "\
	file://0001-add-drp-property-to-devicetree.patch \
	file://0002-enable-drp-driver.patch \
"
