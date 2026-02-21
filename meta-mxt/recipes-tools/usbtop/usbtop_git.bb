SUMMARY = "top-like utility that shows estimated instantaneous bandwidth on USB buses/devices"
HOMEPAGE = "https://github.com/aguinet/usbtop"
LICENSE = "GPL-2.0-only"
LIC_FILES_CHKSUM = "file://LICENSE;md5=44f3f9a1047fceb8541b45e3e4d9baa7"

SRC_URI = "git://github.com/aguinet/usbtop.git;protocol=https;branch=master"
SRCREV = "b9a26bd22b91b84bd72906c6501e61df7b13f3d6"

S = "${WORKDIR}/git"

inherit cmake pkgconfig

DEPENDS += "libusb1 ncurses libpcap"
