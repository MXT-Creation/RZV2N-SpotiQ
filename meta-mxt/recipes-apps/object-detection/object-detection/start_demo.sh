#!/bin/sh

sleep 3

export XDG_RUNTIME_DIR=/run/user/0


echo "Launching demo app..."
cd /home/root/tvm
./object_detection

