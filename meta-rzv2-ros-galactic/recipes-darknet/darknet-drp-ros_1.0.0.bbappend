# Copy library
DEPENDS += " drp-ai-tvm"
RDEPENDS_${PN} += " drp-ai-tvm"

do_configure:prepend() {
    export TVM_HOME="${STAGING_DIR_HOST}/usr/include/tvm"
}

# Apply patches
FILESEXTRAPATHS:prepend := "${THISDIR}/${BPN}:"
SRC_URI += " \
    file://0001-drp-update.patch \
    "
