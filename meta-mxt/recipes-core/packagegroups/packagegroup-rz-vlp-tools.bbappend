# Replace missing uvcvideo module package with Renesas UVCS driver package
RDEPENDS:packagegroup-rz-vlp-tools-multimedia:remove = "kernel-module-uvcvideo"
RDEPENDS:packagegroup-rz-vlp-tools-multimedia:append = " kernel-module-uvcs-drv"
