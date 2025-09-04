#
# This recipe adds Codec_Bin.bin to root/boot.
#

DESCRIPTION = "Recipe for DRPB firmware"
SECTION = "libs"
DEPENDS = ""
LICENSE = "CLOSED"

COMPATIBLE_MACHINE = "rzv2h-family"

CODEC_BIN = "Codec_Bin.bin"

SRC_URI = " \
	file://${CODEC_BIN} \
    "

INSTALL_DIRECTORY ?= "/boot"

do_install() {
    install -d ${D}/${INSTALL_DIRECTORY}/
    install -m 0755 ${WORKDIR}/${CODEC_BIN} ${D}/${INSTALL_DIRECTORY}/${CODEC_BIN}
}

FILES:${PN} = " \
    ${INSTALL_DIRECTORY}/* \
"
