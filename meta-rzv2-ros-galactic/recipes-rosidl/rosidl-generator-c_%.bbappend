FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"

FILES:${PN} += "\
  /usr/lib/lib*.so \
  /usr/lib/rosidl_generator_c \
  /usr/lib/rosidl_generator_c/rosidl_generator_c \
"
SYSROOT_DIRS:append = " \
  /usr/lib \
  /usr/lib/rosidl_generator_c \
  /usr/lib/rosidl_generator_c/rosidl_generator_c \
"

