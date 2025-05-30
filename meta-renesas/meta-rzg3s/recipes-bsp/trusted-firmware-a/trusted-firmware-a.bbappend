require trusted-firmware-a.inc

COMPATIBLE_MACHINE_rzg3s = "(rzg3s-dev|smarc-rzg3s)"

PLATFORM = "g3s"
EXTRA_FLAGS_rzg3s-dev = "BOARD=dev14_1_lpddr PLAT_SYSTEM_SUSPEND=vbat"
EXTRA_FLAGS_smarc-rzg3s = "BOARD=smarc PLAT_SYSTEM_SUSPEND=vbat"

FILES_${PN} = "/boot "
SYSROOT_DIRS += "/boot"

do_compile() {
        oe_runmake PLAT=${PLATFORM} ${EXTRA_FLAGS} bl2 bl31
}

do_install() {
        install -d ${D}/boot
        install -m 644 ${S}/build/${PLATFORM}/release/bl2.bin ${D}/boot/bl2-${MACHINE}.bin
        install -m 644 ${S}/build/${PLATFORM}/release/bl31.bin ${D}/boot/bl31-${MACHINE}.bin
}
