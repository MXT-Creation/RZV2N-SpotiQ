FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

SRC_URI:append:rzg2l-family = " \
        file://0002-Workaround-GPU-driver-remove-power-domains-of-GPU-no.patch \
"
SRC_URI:append:rzv2l-family = " \
        file://0002-Workaround-GPU-driver-remove-power-domains-v2l.patch \
"
