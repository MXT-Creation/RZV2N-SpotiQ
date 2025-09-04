DISTRO_FEATURES:append = " opengles opencl"

require mali-graphics.inc

BB_MULTI_PROVIDER_ALLOWED:append = " virtual/libegl virtual/egl virtual/libgles1 virtual/libgles2 "
