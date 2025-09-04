do_install:append() {
	sed -e "/^After=/s/$/ multi-user.target/g" \
		-i ${D}/${systemd_system_unitdir}/weston.service
}
