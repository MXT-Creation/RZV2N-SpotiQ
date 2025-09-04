MACHINE_FEATURES:remove = "${@bb.utils.contains('DISTRO_FEATURES', 'opengles', '', 'opencl', d)}"
USE_MALI = "${@bb.utils.contains('COMBINED_FEATURES', 'opengles', '1', '', d)}"

# For RZ/G2L (L/LC/V2L/V2H)
PREFERRED_PROVIDER_virtual/libgles1:mali-family = "${@bb.utils.contains('COMBINED_FEATURES', 'opengles', 'mali-library', 'mesa', d)}"
PREFERRED_PROVIDER_virtual/libgles2:mali-family = "${@bb.utils.contains('COMBINED_FEATURES', 'opengles', 'mali-library', 'mesa', d)}"
PREFERRED_PROVIDER_virtual/egl:mali-family = "${@bb.utils.contains('COMBINED_FEATURES', 'opengles', 'mali-library', 'mesa', d)}"
PREFERRED_PROVIDER_virtual/libgbm:mali-family = "${@bb.utils.contains('COMBINED_FEATURES', 'opengles', 'mali-library', 'mesa', d)}"
PREFERRED_PROVIDER_virtual/libopencl:mali-family ?= "${@bb.utils.contains('COMBINED_FEATURES', 'opencl', 'mali-library', '', d)}"
BB_MULTI_PROVIDER_ALLOWED:append:mali-family = " virtual/libgbm "

IMAGE_INSTALL:append:mali-family = " ${@bb.utils.contains('COMBINED_FEATURES', 'opencl', 'libopencl', '', d)}"
IMAGE_INSTALL:append:mali-family = " ${@bb.utils.contains('COMBINED_FEATURES', 'opengles', 'libegl libgles2', '', d)}"
