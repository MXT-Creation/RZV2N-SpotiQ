SUMMARY = "Vosk speech recognition library (prebuilt binary for aarch64)"
HOMEPAGE = "https://alphacephei.com/vosk/"
LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://LICENSE;md5=650b869bd8ff2aed59c62bad2a22a821"

SRC_URI = "\
  https://github.com/alphacep/vosk-api/releases/download/v0.3.45/vosk-linux-aarch64-0.3.45.zip;subdir=vosk-bin;unpack=1;sha256sum=45e95d37755deb07568e79497d7feba8c03aee5a9e071df29961aa023fd94541 \
  https://raw.githubusercontent.com/alphacep/vosk-api/v0.3.45/COPYING;downloadfilename=LICENSE;sha256sum=7c34d28e784b202aa4998f477fd0aa9773146952d7f6fa5971369fcdda59cf48 \
"

S = "${WORKDIR}"

COMPATIBLE_HOST = "aarch64.*-linux"
PACKAGE_ARCH = "${MACHINE_ARCH}"

# Keep unversioned .so in the runtime package (not -dev)
SOLIBS = ".so"
FILES_SOLIBSDEV = ""

do_compile[noexec] = "1"

do_install() {
    install -d ${D}${libdir}
    found="$(find ${WORKDIR}/vosk-bin -name 'libvosk.so' -type f | head -n1)"
    [ -n "$found" ] || bbfatal "libvosk.so not found in extracted archive"
    install -m 0755 "$found" ${D}${libdir}/libvosk.so

    install -d ${D}${datadir}/licenses/${PN}
    install -m 0644 ${WORKDIR}/LICENSE ${D}${datadir}/licenses/${PN}/
}

FILES:${PN} = "${libdir}/libvosk.so ${datadir}/licenses/${PN}"

# Prebuilt binary: silence expected QA checks
INSANE_SKIP:${PN} += "ldflags already-stripped"

