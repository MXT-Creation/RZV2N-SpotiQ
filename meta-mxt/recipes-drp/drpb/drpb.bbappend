# Make sure runtime ships the library (unversioned & any versioned variants)
FILES:${PN} += "${libdir}/libdrp_api.so ${libdir}/libdrp_api.so.*"

# Allow unversioned .so in runtime and help shlib scanner recognize it
INSANE_SKIP:${PN} += "dev-so"
SOLIBS = ".so"
SOLIBSDEV = ".so"

# CRITICAL: explicitly provide the soname DNF requires
RPROVIDES:${PN} += "libdrp_api.so()(64bit)"
