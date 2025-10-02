SUMMARY = "Vosk offline speech recognition models (small en-US)"
HOMEPAGE = "https://alphacephei.com/vosk/"
LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/Apache-2.0;md5=89aea4e17d99a7cacdbeed46a0096b10"

SRC_URI = "https://alphacephei.com/vosk/models/vosk-model-small-en-us-0.15.zip;name=model"
SRC_URI[model.sha256sum] = "30f26242c4eb449f948e42cb302dd7a686cb29a3423a8367f99ff41780942498"

# BitBake unpacks the ZIP into this dir
S = "${WORKDIR}/vosk-model-small-en-us-0.15"

inherit allarch

PACKAGES =+ "${PN}-model-en-us"
FILES:${PN}-model-en-us = "${datadir}/vosk/models/en-us/"

do_configure[noexec] = "1"
do_compile[noexec] = "1"

do_install() {
    install -d ${D}${datadir}/vosk/models/en-us

    cp -r --no-preserve=ownership ${S}/* ${D}${datadir}/vosk/models/en-us/

    # Normalize permissions (optional but clean)
    find ${D}${datadir}/vosk/models/en-us -type d -exec chmod 0755 {} \;
    find ${D}${datadir}/vosk/models/en-us -type f -exec chmod 0644 {} \;
}

# Keep main package empty (optional)
FILES:${PN} = ""
