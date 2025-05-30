FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"

SRC_URI_append = " \
    file://0004-gl-renderer-Fix-build-error-when-platform-does-not-s.patch \
    file://0005-libweston-renderer-gl-Fix-issue-conflict-to-RZ-G1-Po.patch \
    file://0006-backend-drm-Select-plane-based-on-current-attached-C.patch \
"
