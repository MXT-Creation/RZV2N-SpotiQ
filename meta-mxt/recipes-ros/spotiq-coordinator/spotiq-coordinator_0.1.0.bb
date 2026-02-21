DESCRIPTION = "SpotiQ Coordinator - ROS2 robot arm control system for UFACTORY Lite6"
LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/Apache-2.0;md5=89aea4e17d99a7cacdbeed46a0096b10"

inherit ros_distro_humble
inherit ros_ament_python

DEPENDS = " \
    rclpy \
    std-msgs \
    sensor-msgs \
    geometry-msgs \
    darknet-ros-msgs \
    xarm-msgs \
    python3-numpy-native \
"

RDEPENDS:${PN} = " \
    python3-core \
    python3-json \
    python3-threading \
    python3-datetime \
    python3-math \
    python3-numpy \
    rclpy \
    std-msgs \
    sensor-msgs \
    geometry-msgs \
    darknet-ros-msgs \
    xarm-msgs \
"

# Use local files from the recipe directory
SRC_URI = " \
    file://package.xml \
    file://setup.py \
    file://setup.cfg \
    file://resource/spotiq_coordinator \
    file://spotiq_coordinator/__init__.py \
    file://spotiq_coordinator/coordinator_node.py \
    file://launch/coordinator.launch.py \
"

S = "${WORKDIR}"

ROS_BUILD_TYPE = "ament_python"

# Files are already in WORKDIR, ready to build
do_configure:prepend() {
    # Verify setup.py exists
    if [ ! -f ${S}/setup.py ]; then
        bbfatal "setup.py not found in ${S}"
    fi
}

# Standard ament_python install handles most files
# But setup.py creates wrong paths, so we fix them manually
do_install:append() {
    # Remove incorrectly installed share/share directory
    if [ -d ${D}${ros_datadir}/share ]; then
        rm -rf ${D}${ros_datadir}/share
    fi
    
    # Install package.xml to correct location
    install -d ${D}${ros_datadir}/${ROS_BPN}
    install -m 0644 ${S}/package.xml ${D}${ros_datadir}/${ROS_BPN}/
    
    # Install ament index resource marker
    install -d ${D}${ros_datadir}/ament_index/resource_index/packages
    touch ${D}${ros_datadir}/ament_index/resource_index/packages/${ROS_BPN}
    
    # Install launch file
    install -d ${D}${ros_datadir}/${ROS_BPN}/launch
    install -m 0644 ${S}/launch/coordinator.launch.py ${D}${ros_datadir}/${ROS_BPN}/launch/
}

FILES:${PN} += " \
    ${ros_libdir}/${ROS_BPN}/* \
    ${ros_datadir}/${ROS_BPN}/* \
    ${ros_datadir}/ament_index/resource_index/packages/${ROS_BPN} \
"

FILES:${PN}-dev = "${includedir}"
FILES:${PN}-staticdev = ""
