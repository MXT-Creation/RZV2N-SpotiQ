DEPENDS += "octomap"

EXTRA_OECMAKE:remove = "-DFCL_WITH_OCTOMAP=OFF"
EXTRA_OECMAKE:append = " \
  -DFCL_WITH_OCTOMAP:BOOL=ON \
  -DOCTOMAP_INCLUDE_DIR=${RECIPE_SYSROOT}${includedir} \
  -DOCTOMAP_INCLUDE_DIRS=${RECIPE_SYSROOT}${includedir} \
  -DOCTOMAP_LIBRARY=${RECIPE_SYSROOT}${libdir}/liboctomap.so \
  -DOCTOMATH_LIBRARY=${RECIPE_SYSROOT}${libdir}/liboctomath.so \
  -DOCTOMAP_LIBRARIES=${RECIPE_SYSROOT}${libdir}/liboctomap.so\;${RECIPE_SYSROOT}${libdir}/liboctomath.so \
  -DCMAKE_PREFIX_PATH=${RECIPE_SYSROOT}/usr\;${RECIPE_SYSROOT}/opt/ros/humble \
  -DCMAKE_LIBRARY_PATH=${RECIPE_SYSROOT}/usr/lib\;${RECIPE_SYSROOT}/usr/lib64\;${RECIPE_SYSROOT}/opt/ros/humble/lib \
  -DCMAKE_INCLUDE_PATH=${RECIPE_SYSROOT}${includedir}\;${RECIPE_SYSROOT}/opt/ros/humble/include \
"

# Clear stale cache so old OFF/paths can't stick
do_configure:prepend() {
    rm -f ${B}/CMakeCache.txt
}
