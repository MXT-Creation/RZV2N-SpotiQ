LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://LICENSE;md5=86d3f3a95c324c9479bd8986968f4327"

SRC_URI = "git://github.com/morrisfranken/argparse/;protocol=https"
SRCREV = "6bc7076ccb1a6eb7549b2e0b0c213a8226eda02b"

S = "${WORKDIR}/git"

inherit cmake

EXTRA_OECMAKE = ""
