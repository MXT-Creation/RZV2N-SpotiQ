#
# Copyright (c) 2024 IMD Technologies
#

LICENSE = "Proprietary"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/files/common-licenses/Proprietary;md5=0557f9d92cf58f2ccdd50f62f8ac0b28"

SRC_URI = "file://imdt-libhttpserver-v0.1.1.tar.gz"
SRC_URI[sha256sum] = "8f33bec852b122ffb42682d916e77dfddcce81b2150b40c1dc3dcbcc68ad2721"

PV = "0.1.1"

S = "${WORKDIR}/imdt-libhttpserver-v0.1.1"

inherit cmake pkgconfig

EXTRA_OECMAKE = ""

DEPENDS += "openssl glog"
RDEPENDS_${PN} += "openssl"
