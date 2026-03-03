#
# Copyright (c) 2026 MXT
#

FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

# Add yolov8n model
SRC_URI:append = " \
    file://0001-Update-darknet-dpr-ros-to-yolo8n-and-customized-class.patch \
"

# Add std_srvs dependency for model switching service
DEPENDS += "std-srvs"

# Add darknet_ros_msgs dependency (should already be there, but making it explicit)
DEPENDS += "darknet-ros-msgs"

# Add runtime dependency on darknet-drp-model (provides model binaries)
RDEPENDS:${PN} += "darknet-drp-model"

# Ensure proper build order
do_configure[depends] += "darknet-drp-model:do_install"
