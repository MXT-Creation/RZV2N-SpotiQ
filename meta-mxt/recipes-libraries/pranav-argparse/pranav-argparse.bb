LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://LICENSE;md5=4f3ed9ec2c801700ac8fda1fcd29a330"

SRC_URI = "git://github.com/p-ranav/argparse.git;protocol=https"
SRCREV = "af442b4da0cd7a07b56fa709bd16571889dc7fda"

S = "${WORKDIR}/git"

inherit cmake

EXTRA_OECMAKE = ""
