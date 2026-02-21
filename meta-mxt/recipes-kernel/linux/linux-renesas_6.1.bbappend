FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"

KERNEL_DEVICETREE = " \
	renesas/r9a09g056n44-mxt.dtb \
"

SRC_URI:append +=  "\
	file://dts/ \
	file://fragment.cfg \
	file://0001-drivers-media-i2c-tevs-backport-driver-TechNexi.patch \
	file://0002_mipi_dsi_dirty_patch_to_set_LP11_mode.patch \
	file://0003_tc358767.patch \
	file://0004-media-uvc-add-RealSense-custom-pixel-formats-Y16I-RW.patch \
"

do_compile:prepend() {
	cp -rf ${WORKDIR}/dts/* ${S}/arch/arm64/boot/dts/renesas/
}

do_install:append() {
	# This way we get a booting system, even if the camera is not the same
	install -m 0755 -d ${D}/boot
	cp ${D}/boot/r9a09g056n44-mxt.dtb ${D}/boot/r9a09g056n44-evk.dtb
}
