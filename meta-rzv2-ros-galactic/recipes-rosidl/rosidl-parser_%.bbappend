FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"

FILES:${PN} += "\
  /usr/lib/lib*.so \
  /usr/lib/rosidl_parser \
  /usr/lib/rosidl_parser/idl2png \
"
SYSROOT_DIRS:append = " \
  /usr/lib \
  /usr/lib/rosidl_parser \
  /usr/lib/rosidl_parser/idl2png \
"

