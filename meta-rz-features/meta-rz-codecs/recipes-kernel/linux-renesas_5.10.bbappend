#
# Copyright (C) 2024 RenesasElectronics, Co, Ltd.
#

FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}/:"

SRC_URI_append_r9a09g056 += "\
        file://0001-arm64-dts-renesas-r9a09g056-Add-node-and-memory-area.patch \
        file://0003-enable-drp-drv.patch \
"

SRC_URI_append_r9a09g057 = "\
	file://0002-arm64-dts-renesas-r9a09g057-Add-node-and-memory-area.patch \
	file://0023-enable-drp-drv.patch \
"
