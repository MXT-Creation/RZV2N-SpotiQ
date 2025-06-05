FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"

FILES:${PN} += "\
  /usr/lib/lib*.so* \
  /usr/lib/rosidl_typesupport_fastrtps_cpp/rosidl_typesupport_fastrtps_cpp \
  /usr/lib/rosidl_typesupport_fastrtps_cpp/rosidl_typesupport_fastrtps_cpp/* \
"
SYSROOT_DIRS:append = " \
  /usr/lib \
  /usr/lib/rosidl_typesupport_fastrtps_cpp \
  /usr/lib/rosidl_typesupport_fastrtps_cpp/rosidl_typesupport_fastrtps_cpp \
"

