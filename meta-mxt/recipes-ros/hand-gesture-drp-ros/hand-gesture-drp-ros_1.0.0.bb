SUMMARY = "Hand gesture DRP ROS2 node"
DESCRIPTION = "ROS 2 Humble package for hand gesture inference on Renesas DRP-AI."
LICENSE = "CLOSED"

S = "${WORKDIR}"

ROS_PACKAGE_NAME = "hand_gesture_drp_ros"

ROS_BUILD_TYPE = "ament_cmake"

inherit ros_ament_cmake ros_distro_humble

DEPENDS += "\
    ament-cmake-native \
    rclcpp \
    std-msgs \
    sensor-msgs \
    cv-bridge \
    opencv \
    rosidl-adapter \
    drp-ai-tvm \
"

RDEPENDS:${PN} += "\
    drp-ai-tvm \
    hand-gesture-drp-model \
"

# ---------------------------------------------------------------------------
# Sources staged into WORKDIR
# ---------------------------------------------------------------------------
SRC_URI = "\
    file://CMakeLists.txt \
    file://package.xml \
    file://src/ \
    file://include/ \
    file://launch/ \
    file://drp-ai_tvm_v251/ \
    file://config/ \
"

# ---------------------------------------------------------------------------
# Install paths
# ---------------------------------------------------------------------------
ROS_SHARE_DIR = "${datadir}/${ROS_PACKAGE_NAME}"

FILES:${PN} += "\
    ${ROS_SHARE_DIR}/launch/* \
    ${ROS_SHARE_DIR}/config/* \
"

# ---------------------------------------------------------------------------
# Configure
# ---------------------------------------------------------------------------
do_configure:prepend() {
    export TVM_HOME="${STAGING_DIR_HOST}/usr/include/tvm"
}

do_compile:prepend() {
    install -d ${RECIPE_SYSROOT}/usr/include/linux \
               ${RECIPE_SYSROOT}/usr/include/uapi/linux

    # DRP-AI UAPI header (asta îți lipsește)
    if [ -f ${STAGING_KERNEL_DIR}/include/linux/drpai.h ]; then
        cp ${STAGING_KERNEL_DIR}/include/linux/drpai.h ${RECIPE_SYSROOT}/usr/include/linux/
    fi

    if [ -f ${STAGING_KERNEL_DIR}/include/uapi/linux/drpai.h ]; then
        cp ${STAGING_KERNEL_DIR}/include/uapi/linux/drpai.h ${RECIPE_SYSROOT}/usr/include/uapi/linux/
    fi
}

# Ensure proper build order
do_configure[depends] += "hand-gesture-drp-model:do_install"
