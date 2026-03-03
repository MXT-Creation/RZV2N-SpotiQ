FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"

SRC_URI += " \
    file://0001-Update-librealsense-with-patches-from-meta-intel-rea.patch \
    "
DEPENDS = "\
    libusb1 \
    nlohmann-json \
    udev \
"
RRECOMMENDS:${PN} += "kernel-module-uvcvideo"

EXTRA_OECMAKE:append = " \
    -DBUILD_UNIT_TESTS:BOOL=OFF \
    -DBUILD_WITH_OPENMP:BOOL=OFF \
    -DCHECK_FOR_UPDATES:BOOL=OFF \
"

PACKAGECONFIG ??= ""
PACKAGECONFIG[rsusb] = "-DFORCE_RSUSB_BACKEND:BOOL=ON,-DFORCE_RSUSB_BACKEND:BOOL=OFF"
