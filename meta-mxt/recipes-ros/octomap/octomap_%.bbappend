# Ensure shared libraries are built
EXTRA_OECMAKE:append = " -DBUILD_SHARED_LIBS=ON "

# Create the unversioned .so symlinks that linkers/CMake expect
do_install:append() {
    for name in octomap octomath; do
        # If versioned SONAME exists but unversioned is missing, create it
        if ls ${D}${libdir}/lib${name}.so.* >/dev/null 2>&1 && [ ! -e ${D}${libdir}/lib${name}.so ]; then
            # point the unversioned .so at the versioned file in the same dir
            ver=$(basename $(echo ${D}${libdir}/lib${name}.so.* | head -n1))
            ln -sf ${ver} ${D}${libdir}/lib${name}.so
        fi
    done
}

# Make sure the dev package actually ships those .so symlinks (needed for sysroot)
FILES:${PN}-dev:append = " ${libdir}/liboctomap.so ${libdir}/liboctomath.so "
