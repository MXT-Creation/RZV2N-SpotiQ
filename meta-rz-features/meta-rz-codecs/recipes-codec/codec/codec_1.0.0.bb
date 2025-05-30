#
# This recipe adds Codec_Bin.bin to root/boot.
#

DESCRIPTION = "Recipe for Codec"
SECTION = "libs"
DEPENDS = ""
LICENSE = "CLOSED"

COMPATIBLE_MACHINE = "(r9a09g057|r9a09g056)"

CODEC_BIN = "Codec_Bin.bin"

SRC_URI = " \
	file://${CODEC_BIN} \
    "

INSTALL_DIRECTORY ?= "/boot"


do_install() {
    install -d ${D}/${INSTALL_DIRECTORY}/
    install -m 0755 ${WORKDIR}/${CODEC_BIN} ${D}/${INSTALL_DIRECTORY}/${CODEC_BIN}
}

FILES_${PN} = " \
    ${INSTALL_DIRECTORY}/* \
"
