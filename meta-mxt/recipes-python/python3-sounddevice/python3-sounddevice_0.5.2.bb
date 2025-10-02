SUMMARY = "Python bindings for PortAudio (play/record audio)"
HOMEPAGE = "https://python-sounddevice.readthedocs.io/"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://LICENSE;md5=6d5b33fbb7f36a747e1ad68af179ed92"

PYPI_PACKAGE = "sounddevice"
inherit pypi python_setuptools_build_meta

# Latest release as of Oct 2, 2025
SRC_URI[sha256sum] = "c634d51bd4e922d6f0fa5e1a975cc897c947f61d31da9f79ba7ea34dff448b49"

# CFFI builds an extension that links to PortAudio
DEPENDS += "portaudio-v19 python3-cffi-native"
RDEPENDS:${PN} += "portaudio-v19 python3-cffi"

# Help the build system find staged headers/libs
CFLAGS += " -I${STAGING_INCDIR}"
LDFLAGS += " -L${STAGING_LIBDIR}"

# No tests here (module ships example scripts only)
inherit setuptools3-base
