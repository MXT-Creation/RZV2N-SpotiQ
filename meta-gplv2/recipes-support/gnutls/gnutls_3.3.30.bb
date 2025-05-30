require gnutls.inc

SRC_URI += " \
    file://configure.ac-fix-sed-command.patch \
    file://use-pkg-config-to-locate-zlib.patch \
"
SRC_URI[md5sum] = "748f4c194a51ca9f2c02d9b7735262c2"
SRC_URI[sha256sum] = "41d70107ead3de2f12390909a05eefc9a88def6cd1f0d90ea82a7dac8b8effee"
