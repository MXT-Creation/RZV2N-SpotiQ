FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"

FILES:${PN} += "\
  /usr/lib/lib*.so* \
"

SYSROOT_DIRS:append = " \
  /usr/lib \
"

#remove 'tf2-tools'...
ROS_EXEC_DEPENDS = " \
    tf2 \
    tf2-bullet \
    tf2-eigen \
    tf2-eigen-kdl \
    tf2-geometry-msgs \
    tf2-kdl \
    tf2-msgs \
    tf2-py \
    tf2-ros \
    tf2-sensor-msgs \
"

