SUMMARY = "Vosk speech recognition library (prebuilt binary for aarch64)"
HOMEPAGE = "https://alphacephei.com/vosk/"
LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://LICENSE;md5=650b869bd8ff2aed59c62bad2a22a821"

inherit python3-dir

SRC_URI = "\
  https://github.com/alphacep/vosk-api/releases/download/v0.3.45/vosk-linux-aarch64-0.3.45.zip;subdir=vosk-bin;unpack=1;sha256sum=45e95d37755deb07568e79497d7feba8c03aee5a9e071df29961aa023fd94541 \
  https://raw.githubusercontent.com/alphacep/vosk-api/v0.3.45/COPYING;downloadfilename=LICENSE;sha256sum=7c34d28e784b202aa4998f477fd0aa9773146952d7f6fa5971369fcdda59cf48 \
"

S = "${WORKDIR}"

COMPATIBLE_HOST = "aarch64.*-linux"
PACKAGE_ARCH = "${MACHINE_ARCH}"

# Allow unversioned .so in runtime (not -dev)
SOLIBS = ".so"
FILES_SOLIBSDEV = ""

do_compile[noexec] = "1"

do_install() {
    # main lib -> /usr/lib
    install -d ${D}${libdir}
    found="$(find ${WORKDIR}/vosk-bin -name 'libvosk.so' -type f | head -n1)"
    [ -n "$found" ] || bbfatal "libvosk.so not found in extracted archive"
    install -m 0755 "$found" ${D}${libdir}/libvosk.so

    # Python expects: .../site-packages/vosk/libvosk.so
    install -d ${D}${PYTHON_SITEPACKAGES_DIR}/vosk
    install -m 0755 ${D}${libdir}/libvosk.so ${D}${PYTHON_SITEPACKAGES_DIR}/vosk/libvosk.so

    # license
    install -d ${D}${datadir}/licenses/${PN}
    install -m 0644 ${WORKDIR}/LICENSE ${D}${datadir}/licenses/${PN}/LICENSE

    # Defensive: ensure no stray top-level /libvosk.so
    rm -f ${D}${base_libdir}/libvosk.so
    rm -f ${D}/libvosk.so
}

# -------------------------------
# Packaging (Scarthgap overrides)
# -------------------------------
PACKAGES += "${PN}-python"

FILES:${PN} += " \
    ${libdir}/libvosk.so \
    ${datadir}/licenses/${PN}/* \
"

FILES:${PN}-python = " \
    ${PYTHON_SITEPACKAGES_DIR}/vosk/libvosk.so \
"

RDEPENDS:${PN}-python = "${PN}"

# -------------------------------
# QA relaxations for prebuilt binary
# -------------------------------
INSANE_SKIP:${PN} += "already-stripped ldflags"
INSANE_SKIP:${PN}-python += "already-stripped ldflags"

