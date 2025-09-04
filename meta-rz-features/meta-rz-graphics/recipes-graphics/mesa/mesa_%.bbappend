PACKAGECONFIG:remove = "${@bb.utils.contains('COMBINED_FEATURES', 'opengles', 'gles', '', d)}"
PACKAGECONFIG:remove = "${@bb.utils.contains('COMBINED_FEATURES', 'opengles', 'egl', '', d)}"
PACKAGECONFIG:remove = "${@bb.utils.contains('COMBINED_FEATURES', 'opengles', 'gbm', '', d)}"
PACKAGECONFIG:remove = "${@bb.utils.contains('COMBINED_FEATURES', 'opencl', 'opencl', '', d)}"

do_install:append() {
    if [ "${USE_MALI}" = "1" ]; then
        rm -f ${D}/${includedir}/gbm.h
    fi
}
