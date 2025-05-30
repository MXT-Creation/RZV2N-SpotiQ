#!/bin/sh

sleep 3

export XDG_RUNTIME_DIR=/run/user/0

v4l2-ctl --set-ctrl=focus_auto=0
v4l2-ctl --set-ctrl=focus_absolute=200
v4l2-ctl --set-ctrl=exposure_auto=1 
v4l2-ctl --set-ctrl=exposure_absolute=100

echo "Launching demo app..."
cd /home/demo/
./app_yolov2_cam
