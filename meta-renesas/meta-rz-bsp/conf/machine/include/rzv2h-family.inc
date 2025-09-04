require conf/machine/include/soc-family.inc
SOC_FAMILY = "rzv2h-family:mali-family"

# Machine configuration
MACHINE_FEATURES = " opengles opencl hwh264enc hwh264dec hwh265enc hwh265dec"

IMAGE_FSTYPES:append = " tar.bz2 ext4"

# Configuration for ARM Trusted Firmware
EXTRA_IMAGEDEPENDS += " trusted-firmware-a"

# flash writer
EXTRA_IMAGEDEPENDS += " flash-writer"

# Firmware-pack
EXTRA_IMAGEDEPENDS += " firmware-pack"

