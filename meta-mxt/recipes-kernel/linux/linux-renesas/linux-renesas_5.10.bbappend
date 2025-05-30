FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"

KERNEL_DEVICETREE = " \
	renesas/r9a09g056n44-mxt-tevs.dtb \
"

SRC_URI_append +=  "\
	file://dts/ \
	file://fragment.cfg \
	file://patches/0001-drivers-media-i2c-tevs-backport-driver-from-TechNexi.patch \
	file://patches/0002-drm-bridge-display-connector-add-DP-support.patch \
	file://patches/0004-patch-drm-aux-structure-drm-dp-helper-helper.patch \
	file://patches/0005-add-MIPI-DSI-patches_for_tc358767.patch \
	file://patches/0006-define-missing-functions-drm_mipi_dsi.patch \
"

do_compile_prepend() {
	cp -rf ${WORKDIR}/dts/* ${S}/arch/arm64/boot/dts/renesas/
}

do_install_append() {
	# This way we get a booting system, even if the camera is not the same
	install -m 0755 -d ${D}/boot
	cp ${D}/boot/r9a09g056n44-mxt-tevs.dtb ${D}/boot/r9a09g056n44-evk.dtb
}
