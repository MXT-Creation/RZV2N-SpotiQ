#
# Copyright (C) 2024 RenesasElectronics, Co, Ltd.
#
DESCRIPTION = "DRP-AI Driver with kernel for RZ/V2N"

FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}/:"

SRC_URI_append += "\
	file://0000-ai_sdk_memory_map.patch \
	file://0001-enable-u-dma-buf-function.patch \
	file://0002_add_drpai_prop_to_devicetree.patch \
	file://0003_add_drpai_drv.patch \
"