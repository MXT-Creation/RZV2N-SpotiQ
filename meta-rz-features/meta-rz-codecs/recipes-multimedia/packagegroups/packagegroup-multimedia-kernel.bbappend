RDEPENDS:packagegroup-multimedia-kernel-modules:append = " \
    ${@oe.utils.conditional("USE_VIDEO_OMX", "1", "kernel-module-uvcs-drv", "", d )} \
"
