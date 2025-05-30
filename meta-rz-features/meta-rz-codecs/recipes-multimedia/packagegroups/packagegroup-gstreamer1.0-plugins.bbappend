DEPENDS += "${@bb.utils.contains("USE_VIDEO_OMX", "1", "gstreamer1.0-omx", "", d)}"

RDEPENDS_packagegroup-gstreamer1.0-plugins += " \
	gstreamer1.0-plugin-vspmfilter \
	${@bb.utils.contains("USE_VIDEO_OMX", "1", "gstreamer1.0-omx", "", d)} \
"

RDEPENDS_packagegroup-gstreamer1.0-plugins_append_rzg2h = " \
	${@bb.utils.contains("USE_VIDEO_OMX", "1", "gstreamer1.0-plugin-vspfilter vspfilter-init", "", d)} \
"
