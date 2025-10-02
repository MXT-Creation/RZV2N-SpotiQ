#
# Copyright (c) 2025 MXT
#

LICENSE = "CLOSED"
LIC_FILES_CHKSUM = ""

inherit pkgconfig systemd

SRC_URI = "file://start_demo.sh \
           file://object-detection.service \
           file://tvm\
"
S = "${WORKDIR}"

DEPENDS = "nlohmann-json pranav-argparse v4l-utils mmngr-user-module mmngrbuf-user-module opencv drpai spdlog"

RDEPENDS:${PN}      += "lib-tvm"
RDEPENDS:${PN}-dev  += "lib-tvm-dev"

# Ship exactly what we install (systemd unit, libtvm, and the /home/root content)
FILES:${PN} += " \
    ${systemd_unitdir}/system/object-detection.service \
    /home/root/start_demo.sh \
    /home/root/tvm \
    /home/root/tvm/* \
"

# Disable QA checks for non-symlink .so and dev-dependency
INSANE_SKIP:${PN} += "dev-so dev-deps file-rdeps"

do_configure[noexec] = "1"
do_compile[noexec] = "1"

do_install:append() {
    # /home/root content
    install -d ${D}/home/root ${D}/home/root/tvm
    install -m 0755 ${WORKDIR}/start_demo.sh ${D}/home/root/
    cp -r ${WORKDIR}/tvm/* ${D}/home/root/tvm/
    chmod -R 0755 ${D}/home/root/tvm/object_detection || true

    # systemd unit
    install -d ${D}${systemd_unitdir}/system
    install -m 0644 ${WORKDIR}/object-detection.service ${D}${systemd_unitdir}/system/
}

SYSTEMD_AUTO_ENABLE = "enable"
SYSTEMD_SERVICE:${PN} = "object-detection.service"
