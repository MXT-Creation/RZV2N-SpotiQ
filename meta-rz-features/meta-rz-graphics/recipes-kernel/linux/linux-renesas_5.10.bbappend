FILESEXTRAPATHS_prepend := "${THISDIR}/files:"

SRC_URI_append_rzg2l = " \
        file://0002-Workaround-GPU-driver-remove-power-domains-of-GPU-no.patch \
"
SRC_URI_append_rzv2l = " \
        file://0002-Workaround-GPU-driver-remove-power-domains-v2l.patch \
"
