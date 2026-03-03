#!/bin/bash
# ============================================================
# RZV2N Yocto - Hand Gesture Node Startup Script
# ============================================================

set -e

# ---- GPIO LED Setup ----
# P60 = gpio-464 (running indicator)
# P64 = gpio-468 (done indicator)

GPIO_RUNNING=464
GPIO_DONE=468

led_init() {
    echo "[INFO] Initializing GPIO LEDs..."
    echo $GPIO_RUNNING > /sys/class/gpio/export 2>/dev/null || true
    echo $GPIO_DONE    > /sys/class/gpio/export 2>/dev/null || true
    sleep 0.2
    echo out > /sys/class/gpio/P60/direction
    echo out > /sys/class/gpio/P64/direction
}

led_running() {
    echo 1 > /sys/class/gpio/P60/value   # P60 ON
    echo 0 > /sys/class/gpio/P64/value   # P64 OFF
    echo "[INFO] LED: running (P60 ON)"
}

led_done() {
    echo 0 > /sys/class/gpio/P60/value   # P60 OFF
    echo 1 > /sys/class/gpio/P64/value   # P64 ON
    echo "[INFO] LED: done (P64 ON)"
}

# Initialize and turn on running LED
led_init
led_running

# ---- Change MAC Address on end0 ----
echo "[INFO] Bringing end0 down to change MAC address..."
ip link set dev end0 down
sleep 1

echo "[INFO] Setting MAC address to 02:22:33:44:55:66..."
ip link set dev end0 address 02:22:33:44:55:66
sleep 1

echo "[INFO] Bringing end0 back up..."
ip link set dev end0 up
sleep 2

# ---- USB Memory Configuration ----
echo "[INFO] Setting USB filesystem memory limit to 500MB..."
echo 500 | tee /sys/module/usbcore/parameters/usbfs_memory_mb
sleep 1

# ---- Source ROS 2 Humble Environment ----
echo "[INFO] Sourcing ROS 2 Humble setup..."
source /opt/ros/humble/setup.bash
sleep 1

# ---- Stop existing ROS 2 daemon ----
echo "[INFO] Stopping ROS 2 daemon..."
ros2 daemon stop
sleep 2

# ---- Launch Hand Gesture Recognition ----
led_done
echo "[INFO] Launching hand gesture recognition node..."
ros2 launch hand_gesture_drp_ros gesture_recognition.launch.py
sleep 5

# ---- Node exited - switch to done LED ----
echo "[INFO] Hand gesture node exited."
