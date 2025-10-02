# meta-yourlayer/recipes-python/vosk/python3-vosk_0.3.50.bb
SUMMARY = "Offline speech recognition API (Python bindings)"
HOMEPAGE = "https://github.com/alphacep/vosk-api"
LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://../COPYING;md5=d09bbd7a3746b6052fbd78b26a87396b"

SRC_URI = "https://github.com/alphacep/vosk-api/archive/refs/tags/v0.3.50.tar.gz \
           file://0001-Disable-runtime-dep-check-in-setup.py.patch \
"

SRC_URI[sha256sum] = "cc1067bcc599c9a2f5f38d4257caf2ac636ba244f7c965cee20293a41024f70f"

S = "${WORKDIR}/vosk-api-0.3.50/python"

inherit python_setuptools_build_meta

# CRITICAL: prevent pip from trying to resolve/install deps during build
PYPA_WHEEL_ARGS += " --no-deps --no-build-isolation"
PYPA_BUILD_ARGS += " --no-deps --no-build-isolation"

DEPENDS += "python3-cffi-native"

RDEPENDS:${PN} += "\
    python3-core \
    python3-cffi \
    python3-requests \
    python3-tqdm \
    python3-srt \
    python3-websockets \
"
