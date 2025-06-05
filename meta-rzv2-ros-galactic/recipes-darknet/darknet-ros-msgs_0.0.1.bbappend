
# Apply patches
FILESEXTRAPATHS:prepend := "${THISDIR}/${BPN}:"
SRC_URI += " \
    file://0001-msg-udpate-cmake.patch \
    "
