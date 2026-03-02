#
# Copyright (c) 2026 MXT
#

LICENSE = "CLOSED"
LIC_FILES_CHKSUM = ""

inherit systemd

SRC_URI = "\
           file://start-handgesture.service \
           file://start_gesture_node.sh \
"

RDEPENDS:${PN} += "bash"

do_install:append() {
    install -d ${D}/home
    
    install -m 0755 -d ${D}/home
    install -m 0755 ${WORKDIR}/start_gesture_node.sh ${D}/home/start_gesture_node.sh
    
    install -m 0755 -d ${D}${systemd_unitdir}/system
    install -m 0644 ${WORKDIR}/start-handgesture.service ${D}${systemd_unitdir}/system/start-handgesture.service
}

FILES:${PN} += " \
    /home/start_gesture_node.sh \
    ${systemd_unitdir}/system/start-handgesture.service \
"

SYSTEMD_AUTO_ENABLE = "disable"
SYSTEMD_SERVICE_${PN} = "start-handgesture.service"
