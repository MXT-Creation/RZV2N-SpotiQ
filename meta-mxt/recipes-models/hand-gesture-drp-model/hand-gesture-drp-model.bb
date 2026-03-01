#
# Copyright (c) 2025 MXT
#

SUMMARY = "Install darknet DRP ROS assets to /hand_gesture_drp_ros"
LICENSE = "CLOSED"

SRC_URI = "file://hand-gesture-drp-model.tar.gz"
S = "${WORKDIR}/hand-gesture-drp-model"

do_configure[noexec] = "1"
do_compile[noexec] = "1"

do_install() {
    install -d ${D}/hand_gesture_drp_ros

    # Option A: cp without preserving ownership (preferred & simple)
    cp -R --no-preserve=ownership ${S}/. ${D}/hand_gesture_drp_ros/

    # Ensure root:root ownership and reasonable perms (pseudo will record as root)
    chown -R root:root ${D}/hand_gesture_drp_ros
}

FILES:${PN} += " /hand_gesture_drp_ros /hand_gesture_drp_ros/* /hand_gesture_drp_ros/** "
