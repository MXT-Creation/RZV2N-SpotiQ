PNBLACKLIST[navigation2] = ""
PNBLACKLIST[nav2-bringup] = ""
IMAGE_INSTALL:append = " navigation2 nav2-bringup"

## X11 not support...
RDEPENDS:${PN}:remove = " nav2-rviz-plugins"

# OPEN-MP not support...
RDEPENDS:${PN}:remove = " nav2-smac-planner"
