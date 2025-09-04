require trusted-firmware-a-renesas.inc

COMPATIBLE_MACHINE = "(rzg3e-family|rzv2h-family)"

LIC_FILES_CHKSUM = "file://${WORKDIR}/git/docs/license.rst;md5=b2c740efedc159745b9b31f88ff03dde"
PV = "2.7+git${SRCPV}"

S = "${WORKDIR}/git"

TFA_URI ?= "git://github.com/renesas-rz/rzg_trusted-firmware-a.git;protocol=https"
TFA_REV ?= "ac42ae2d33a0770c5ee160ab1b2fe263e67bd680"
TFA_BRANCH ?= "2.7.0/rz_dev"

# for RZ/V2N
SRC_URI = "${TFA_URI};nobranch=1"
SRCREV = "${TFA_REV}"

BUILD_DIR = "${B}/${TFA_PLATFORM}"
BUILD_DIR .= "/${@'debug' if d.getVar("TFA_DEBUG") == '1' else 'release'}"

do_compile:prepend() {
	# This is still needed to have the native tools executing properly by
	# setting the RPATH
	sed -i '/^LDLIBS/ s,$, \$\{BUILD_LDFLAGS},' ${S}/tools/fiptool/Makefile
}

do_deploy[noexec] = "1"
