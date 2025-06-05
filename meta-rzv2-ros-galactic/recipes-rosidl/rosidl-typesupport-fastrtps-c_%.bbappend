FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"

FILES:${PN} += "\
  /usr/lib/lib*.so* \
  /usr/lib/rosidl_typesupport_fastrtps_c/* \
  /usr/lib/rosidl_typesupport_fastrtps_c/rosidl_typesupport_fastrtps_c/* \
"
SYSROOT_DIRS:append = " \
  /usr/lib \
  /usr/lib/rosidl_typesupport_fastrtps_c \
  /usr/lib/rosidl_typesupport_fastrtps_c/rosidl_typesupport_fastrtps_c \
"

