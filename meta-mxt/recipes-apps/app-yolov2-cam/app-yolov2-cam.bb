#
# Copyright (c) 2025 MXT
#

LICENSE = "CLOSED"
LIC_FILES_CHKSUM = ""

inherit cmake pkgconfig systemd

SRC_URI = "file://src \
           file://app-yolo2-cam.service \
           file://start_demo.sh \
           file://yolov2_cam \
"

S = "${WORKDIR}//src"

DEPENDS = "nlohmann-json pranav-argparse v4l-utils mmngr-user-module mmngrbuf-user-module opencv drpai spdlog i2c-tools"
RDEPENDS_${PN} = "kernel-module-mmngr kernel-module-mmngrbuf bash"

FILES_${PN}_append = " \
    ${systemd_unitdir}/* \
    /home/demo/app_yolov2_cam \
    /home/demo/yolov2_cam/* \
    /home/demo/ \
"

do_install_append() {
    install -d ${D}/home/demo
    
    install -m 0755 -d ${D}/home/demo
    install -m 0755 ${WORKDIR}/start_demo.sh ${D}/home/demo
    
    cp -r ${WORKDIR}/yolov2_cam ${D}/home/demo/
    chmod -R 0755 ${D}/home/demo/yolov2_cam

    install -m 0755 -d ${D}${systemd_unitdir}/system
    install -m 0644 ${WORKDIR}/app-yolo2-cam.service ${D}${systemd_unitdir}/system/
}

SYSTEMD_AUTO_ENABLE = "enable"
SYSTEMD_SERVICE_${PN} = "app-yolo2-cam.service"
