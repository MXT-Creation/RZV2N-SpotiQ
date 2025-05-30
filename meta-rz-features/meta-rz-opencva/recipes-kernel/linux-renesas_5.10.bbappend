#
# Copyright (C) 2024 RenesasElectronics, Co, Ltd.
#
DESCRIPTION = "RZ/V2N OpenCVA Package"

FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}/:"

SRC_URI_append += "\
	file://0002-add-drp-prop-to-devicetree.patch \
	file://0003-enable-drp-drv.patch\
"
