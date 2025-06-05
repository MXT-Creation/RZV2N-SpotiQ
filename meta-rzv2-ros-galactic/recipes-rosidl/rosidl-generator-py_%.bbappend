FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"

FILES:${PN} += "\
  /usr/lib/lib*.so \
  /usr/lib/rosidl_generator_py \
  /usr/lib/rosidl_generator_py/rosidl_generator_py \
"
SYSROOT_DIRS:append = " \
  /usr/lib \
  /usr/lib/rosidl_generator_py \
  /usr/lib/rosidl_generator_py/rosidl_generator_py \
"

