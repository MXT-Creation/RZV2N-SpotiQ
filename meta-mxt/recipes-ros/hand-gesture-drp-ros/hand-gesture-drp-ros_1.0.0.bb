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
    file://model/ \
    file://config/ \
"

# ---------------------------------------------------------------------------
# Install paths
# ---------------------------------------------------------------------------
ROS_SHARE_DIR = "${datadir}/${ROS_PACKAGE_NAME}"

# ---------------------------------------------------------------------------
# Model subpackage
# ---------------------------------------------------------------------------
PACKAGES += "${PN}-model"

FILES:${PN} += "\
    ${ROS_SHARE_DIR}/launch/* \
"

FILES:${PN}-model += "\
    /hand_gesture_drp_ros/hand_yolov3_onnx/deploy.so \
    /hand_gesture_drp_ros/hand_yolov3_onnx/deploy.params \
    /hand_gesture_drp_ros/hand_yolov3_onnx/deploy.json \
    /hand_gesture_drp_ros/labels.txt \
"

INSANE_SKIP:${PN}-model += "already-stripped arch"
INHIBIT_PACKAGE_STRIP_FILES = "/hand_gesture_drp_ros/hand_yolov3_onnx/deploy.so"

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

# ---------------------------------------------------------------------------
# Install
# ---------------------------------------------------------------------------
do_install:append() {
    # Launch files
    if [ -d "${S}/launch" ]; then
        install -d ${D}${ROS_SHARE_DIR}/launch
        cp -r --no-dereference --preserve=mode,links,timestamps \
            ${S}/launch/* ${D}${ROS_SHARE_DIR}/launch/ || true
    fi

    # Model artefacts -> /hand_gesture_drp_ros/ (paths match define.h)
    if [ -d "${S}/model" ] && [ "$(ls -A ${S}/model 2>/dev/null)" ]; then
        install -d ${D}/hand_gesture_drp_ros/hand_yolov3_onnx
        for f in deploy.so deploy.params deploy.json; do
            [ -f "${S}/model/${f}" ] && \
                install -m 0755 ${S}/model/${f} \
                    ${D}/hand_gesture_drp_ros/hand_yolov3_onnx/${f} || true
        done
        [ -f "${S}/model/labels.txt" ] && \
            install -m 0644 ${S}/model/labels.txt \
                ${D}/hand_gesture_drp_ros/labels.txt || true
    fi
}
