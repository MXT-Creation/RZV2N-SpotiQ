FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"

FILES:${PN} += "\
  /usr/lib/lib*.so \
  /usr/lib/rosidl_generator_cpp \
  /usr/lib/rosidl_generator_cpp/rosidl_generator_cpp \
"
SYSROOT_DIRS:append = " \
  /usr/lib \
  /usr/lib/rosidl_generator_cpp \
  /usr/lib/rosidl_generator_cpp/rosidl_generator_cpp \
"

