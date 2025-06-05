do_install() {
    oe_runmake install INSTALL=${D}${prefix} INSTALL_LIB=${D}${prefix}${base_libdir}
}

