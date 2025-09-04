IMAGE_INSTALL:append = " \
	kernel-module-mmngr \
	mmngrbuf-user-module \
	mmngr-user-module \
	libjpeg-turbo-dev \
	opencv \
	opencv-dev \
	sudo \
	lib-tvm \
"

TOOLCHAIN_TARGET_TASK:append = " drpai "

update_issue() {
    AI_SDK_VERSION="AI SDK V6.00 (Source Code)"

    case "${MACHINE}" in
    rzv2n-evk)
      AI_SDK_BOARD="RZ/V2N EVK"
      AI_SDK_SOC="RZ/V2N"
      ;;
    rzv2n-dev)
      AI_SDK_BOARD="RZ/V2N DEV"
      AI_SDK_SOC="RZ/V2N"
      ;;
    esac

    echo "BOARD: ${AI_SDK_BOARD}" >> ${IMAGE_ROOTFS}/etc/issue
    echo "LSI: ${AI_SDK_SOC}" >> ${IMAGE_ROOTFS}/etc/issue
    echo "${AI_SDK_VERSION}" >> ${IMAGE_ROOTFS}/etc/issue
}
ROOTFS_POSTPROCESS_COMMAND += "update_issue; "

enable_sudo() {
    echo "weston ALL=(ALL:ALL) NOPASSWD: ALL" > ${IMAGE_ROOTFS}/etc/sudoers.d/weston
}
ROOTFS_POSTPROCESS_COMMAND += "enable_sudo;"
