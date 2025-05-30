MACHINE_FEATURES_remove = "${@bb.utils.contains('DISTRO_FEATURES', 'opengles', '', 'opencl', d)}"
USE_MALI = "${@bb.utils.contains('COMBINED_FEATURES', 'opengles', '1', '', d)}"

# For RZ/G2L (L/LC/V2L)

PREFERRED_PROVIDER_virtual/libgles1_rzg2l = "${@bb.utils.contains('COMBINED_FEATURES', 'opengles', 'mali-library', 'mesa', d)}"
PREFERRED_PROVIDER_virtual/libgles2_rzg2l = "${@bb.utils.contains('COMBINED_FEATURES', 'opengles', 'mali-library', 'mesa', d)}"
PREFERRED_PROVIDER_virtual/egl_rzg2l = "${@bb.utils.contains('COMBINED_FEATURES', 'opengles', 'mali-library', 'mesa', d)}"
PREFERRED_PROVIDER_virtual/libgbm_rzg2l = "${@bb.utils.contains('COMBINED_FEATURES', 'opengles', 'mali-library', 'mesa', d)}"
PREFERRED_PROVIDER_virtual/libopencl_rzg2l ?= "${@bb.utils.contains('COMBINED_FEATURES', 'opencl', 'mali-library', '', d)}"
MULTI_PROVIDER_WHITELIST_append_rzg2l = " virtual/libgbm "

IMAGE_INSTALL_append_rzg2l = " ${@bb.utils.contains('COMBINED_FEATURES', 'opencl', 'libopencl', '', d)}"
IMAGE_INSTALL_append_rzg2l = " ${@bb.utils.contains('COMBINED_FEATURES', 'opengles', 'libegl libgles2', '', d)}"

BBMASK_append_rzg2l = " recipes-graphics/powervr|recipes-kernel/kernel-module-powervr "

# For RZ/V2H

PREFERRED_PROVIDER_virtual/libgles1_rzv2h = "${@bb.utils.contains('COMBINED_FEATURES', 'opengles', 'mali-library', 'mesa', d)}"
PREFERRED_PROVIDER_virtual/libgles2_rzv2h = "${@bb.utils.contains('COMBINED_FEATURES', 'opengles', 'mali-library', 'mesa', d)}"
PREFERRED_PROVIDER_virtual/egl_rzv2h = "${@bb.utils.contains('COMBINED_FEATURES', 'opengles', 'mali-library', 'mesa', d)}"
PREFERRED_PROVIDER_virtual/libgbm_rzv2h = "${@bb.utils.contains('COMBINED_FEATURES', 'opengles', 'mali-library', 'mesa', d)}"
PREFERRED_PROVIDER_virtual/libopencl_rzv2h ?= "${@bb.utils.contains('COMBINED_FEATURES', 'opencl', 'mali-library', '', d)}"
MULTI_PROVIDER_WHITELIST_append_rzv2h = " virtual/libgbm "

IMAGE_INSTALL_append_rzv2h = " ${@bb.utils.contains('COMBINED_FEATURES', 'opencl', 'libopencl', '', d)}"
IMAGE_INSTALL_append_rzv2h = " ${@bb.utils.contains('COMBINED_FEATURES', 'opengles', 'libegl libgles2', '', d)}"

BBMASK_append_rzv2h = " recipes-graphics/powervr|recipes-kernel/kernel-module-powervr "
