#
# Copyright (c) 2025 MXT
#

SUMMARY = "Install darknet DRP ROS assets to /darknet_drp_ros"
LICENSE = "CLOSED"

SRC_URI = "file://darknet-drp-app.tar.gz"
S = "${WORKDIR}/darknet-drp-app"

do_configure[noexec] = "1"
do_compile[noexec] = "1"

do_install() {
    install -d ${D}/darknet_drp_ros

    # Option A: cp without preserving ownership (preferred & simple)
    cp -R --no-preserve=ownership ${S}/. ${D}/darknet_drp_ros/

    # Ensure root:root ownership and reasonable perms (pseudo will record as root)
    chown -R root:root ${D}/darknet_drp_ros
}

FILES:${PN} += " /darknet_drp_ros /darknet_drp_ros/* /darknet_drp_ros/** "
