SUMMARY = "ROS 2 STT node using Vosk (offline speech recognition)"
HOMEPAGE = "https://github.com/bob-ros2/voskros"
LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://LICENSE;md5=c362791cb53e4cb0b5e52d1aba4af336"

SRC_URI = "git://github.com/bob-ros2/voskros.git;branch=main;protocol=https"
S = "${WORKDIR}/git"

PV = "1.0+git${SRCPV}"
SRCREV = "${AUTOREV}"

inherit ros_ament_cmake ros_distro_humble

DEPENDS += "\
    ament-cmake \
    ament-cmake-python \
    ament-cmake-libraries \
    ament-package-native \
    ament-cmake-export-definitions \
    ament-cmake-export-include-directories \
    ament-cmake-export-interfaces \
    ament-cmake-export-libraries \
    ament-cmake-export-link-flags \
    ament-cmake-export-targets \
    ament-cmake-gen-version-h \
    ament-cmake-target-dependencies \
    ament-cmake-include-directories \
    ament-cmake-test \
    ament-cmake-version \
    python3-catkin-pkg-native \
    std-msgs \
    rclcpp \
    rclpy \
    rosidl-adapter \
    rosidl-default-generators-native \
    rosidl-adapter-native \
    rosidl-parser-native \
    rosidl-generator-c-native \
    rosidl-generator-cpp-native \
    rosidl-generator-py-native \
"

RDEPENDS:${PN} += " \
    python3-vosk \
    python3-sounddevice \
    portaudio-v19 \
    alsa-lib \
    rclpy \
    bash \
"

# Packaging
FILES:${PN} += "${ros_prefix}/*"
