do_install:append:mali-family() {
	# These files are provided in other recipes
	rm -rf ${D}/${libdir}/libwayland-egl*
	rm -rf ${D}/${libdir}/pkgconfig/wayland-egl*
}

