FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"

FILES:${PN} += "\
  /usr/lib/lib*.so \
  /usr/lib/rosidl_adapter/msg2idl.py \
  /usr/lib/rosidl_adapter/srv2idl.py \
  /usr/share/rosidl_adapter/cmake/* \
"

SYSROOT_DIRS:append = " \
  /usr/lib \
  /usr/lib/rosidl_adapter \
  /usr/share \
  /usr/share/rosidl_adapter \
  /usr/share/rosidl_adapter/cmake \
"

