DISTRO_FEATURES_append = " hwh264dec hwh264enc hwh265dec hwh265enc "
MACHINE_FEATURES_append = " hwh264dec hwh264enc hwh265dec hwh265enc "

require omx-control.inc

LICENSE_FLAGS_WHITELIST_append=" \
	${@bb.utils.contains("USE_VIDEO_OMX", "1", "commercial_gstreamer1.0-omx", "", d )} \
	${@bb.utils.contains("USE_VIDEO_OMX", "1", "commercial_lib32-libomxil", "", d )} \
	${@bb.utils.contains("USE_VIDEO_OMX", "1", "commercial_libomxil", "", d )} \
"

IMAGE_INSTALL_append =" \
	${@bb.utils.contains("USE_VIDEO_OMX", "1", " kernel-module-uvcs-drv ", "", d)} \
	${@bb.utils.contains("USE_VIDEO_OMX", "1", " codec-user-module ", "",d)} \
"
