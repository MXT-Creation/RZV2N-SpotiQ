FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"

IWLWIFI_FIRMWARE = "https://git.kernel.org/pub/scm/linux/kernel/git/iwlwifi/linux-firmware.git/plain/iwlwifi-cc-a0-46.ucode;md5sum=babe453e0bc18ec93768ec6f002d8229;downloadfilename=iwlwifi-cc-a0-46.ucode"

SRC_URI_append = " \
	${@bb.utils.contains('MACHINE_FEATURES', 'ax200-wifi', '${IWLWIFI_FIRMWARE}', '',d)} \
	${@bb.utils.contains('MACHINE_FEATURES', 'ax200-wifi', 'file://ax200-wifi.cfg', '',d)} \
	${@bb.utils.contains('MACHINE_FEATURES', 'atheros-ar9287-wifi', 'file://atheros-ar9287-wifi.cfg', '',d)} \
	${@bb.utils.contains('MACHINE_FEATURES', 'rtl8169-firmware', 'file://rtl8169-firmware.cfg', '',d)} \
	${@bb.utils.contains('MACHINE_FEATURES', 'nvme', 'file://nvme.cfg', '',d)} \
 "

BUILTIN_FIRMWARE_DIR = "${STAGING_KERNEL_DIR}/drivers/base/firmware_loader/builtin"
do_download_firmware () {
	if [ -f ${WORKDIR}/iwlwifi-cc-a0-46.ucode ]; then
		install -m 755 ${WORKDIR}/iwlwifi-cc-a0-46.ucode ${BUILTIN_FIRMWARE_DIR}
	fi
}

addtask do_download_firmware after do_configure before do_compile
