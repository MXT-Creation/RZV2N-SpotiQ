FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"

SRC_URI_append= " \
        file://0001-usb-xhci-add-code-to-extend-memory-access-to-over-32.patch \
        file://0002-net-ethernet-renesas-ravb-add-code-to-access-over-32.patch \
        file://0003-arm64-boot-dts-renesas-r9a09g011-v2mevk2-enable-ethe.patch \
"
