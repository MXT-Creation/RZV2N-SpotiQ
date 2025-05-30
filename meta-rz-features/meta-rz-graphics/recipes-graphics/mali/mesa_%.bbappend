do_install_append() {
    if [ "${USE_MALI}" = "1" ]; then
        rm -f ${D}/${includedir}/gbm.h
    fi
}
