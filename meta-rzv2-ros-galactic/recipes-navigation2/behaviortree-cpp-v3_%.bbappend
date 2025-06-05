FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"

FILES:${PN} += "\
  /usr/lib/lib*.so* \
  /usr/lib/BehaviorTreeV3/* \
"

SYSROOT_DIRS:append = " \
  /usr/lib \
  /usr/lib/BehaviorTreeV3 \
"

