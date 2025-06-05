require recipes-devtools/gcc/gcc-${PV}.inc
B = "${WORKDIR}/gcc-${PV}/build.${HOST_SYS}.${TARGET_SYS}"
require recipes-devtools/gcc/libgfortran.inc

