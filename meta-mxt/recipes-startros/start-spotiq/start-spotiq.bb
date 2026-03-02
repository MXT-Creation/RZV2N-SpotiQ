#
# Copyright (c) 2026 MXT
#

LICENSE = "CLOSED"
LIC_FILES_CHKSUM = ""

inherit systemd

SRC_URI = "\
           file://start-spotiq.service \
           file://start_spotiq.sh \
"

RDEPENDS:${PN} += "bash"

do_install:append() {
    install -d ${D}/home
    
    install -m 0755 -d ${D}/home
    install -m 0755 ${WORKDIR}/start_spotiq.sh ${D}/home/start_spotiq.sh
    
    install -m 0755 -d ${D}${systemd_unitdir}/system
    install -m 0644 ${WORKDIR}/start-spotiq.service ${D}${systemd_unitdir}/system/start-spotiq.service
}

FILES:${PN} += " \
    /home/start_spotiq.sh \
    ${systemd_unitdir}/system/start-spotiq.service \
"

SYSTEMD_AUTO_ENABLE = "disable"
SYSTEMD_SERVICE_${PN} = "start-spotiq.service"
