IMAGE_INSTALL:append = " \
		packagegroup-gstreamer1.0-plugins \
		packagegroup-multimedia-libs \
		packagegroup-multimedia-kernel-modules \
"

TOOLCHAIN_TARGET_TASK:append = " packagegroup-multimedia-kernel-modules-dev"
