SUMMARY = "Fast, Extensible Progress Meter"
HOMEPAGE = "https://tqdm.github.io/"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://LICENCE;md5=42dfa9e8c616dbc295df3f58d756b2a1"

PYPI_PACKAGE = "tqdm"
inherit pypi python_setuptools_build_meta

SRC_URI[sha256sum] = "e4d936c9de8727928f3be6079590e97d9abfe8d39a590be678eb5919ffc186bb"

S = "${WORKDIR}/${PYPI_PACKAGE}-${PV}"

# Build-time (native) deps for versioning via setuptools_scm and its [toml] extra
DEPENDS += "python3-setuptools-scm-native python3-tomli-native"

# Runtime deps
RDEPENDS:${PN} += "python3-core"

