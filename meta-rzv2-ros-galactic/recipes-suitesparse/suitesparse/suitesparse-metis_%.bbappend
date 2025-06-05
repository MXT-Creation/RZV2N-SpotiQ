do_install:append() {
    mkdir ${D}${libdir}
    install -D -m0644 ${D}${nonarch_libdir}/libmetis.so ${D}${libdir}/libmetis.so
    rm -rf ${D}${nonarch_libdir}
}

