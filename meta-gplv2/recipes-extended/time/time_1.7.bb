SUMMARY = "Tool that measures CPU resources"
DESCRIPTION = "time measures many of the CPU resources, such as time and \
memory, that other programs use."
HOMEPAGE = "http://www.gnu.org/software/time/"
SECTION = "utils"
LICENSE = "GPLv2+"
LIC_FILES_CHKSUM = "file://COPYING;md5=8ca43cbc842c2336e835926c2166c28b"

inherit texinfo update-alternatives

ALTERNATIVE_${PN} = "time"
ALTERNATIVE_PRIORITY = "100"

BBCLASSEXTEND = "native nativesdk"

PR = "r2"

SRC_URI = "${GNU_MIRROR}/time/time-${PV}.tar.gz \
	   file://debian.patch"

SRC_URI[md5sum] = "e38d2b8b34b1ca259cf7b053caac32b3"
SRC_URI[sha256sum] = "e37ea79a253bf85a85ada2f7c632c14e481a5fd262a362f6f4fd58e68601496d"

inherit autotools
