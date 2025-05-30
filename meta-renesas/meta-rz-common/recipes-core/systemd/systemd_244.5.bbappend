FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"

SRC_URI_append += " \
	file://0001-journald-Retry-if-posix_fallocate-returned-1-EINTR-.patch \
"
