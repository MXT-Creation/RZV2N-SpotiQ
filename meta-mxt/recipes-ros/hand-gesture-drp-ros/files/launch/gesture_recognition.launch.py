"""
Launch file for Hand Gesture Recognition ROS2 node.

Usage:
    ros2 launch hand_gesture_drp_ros gesture_recognition.launch.py
    ros2 launch hand_gesture_drp_ros gesture_recognition.launch.py drpai_freq:=3
    ros2 launch hand_gesture_drp_ros gesture_recognition.launch.py camera_device:=/dev/video2
"""

from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node


def generate_launch_description():

    # ── Declare launch arguments ──────────────────────────────────────────────
    declare_drpai_freq = DeclareLaunchArgument(
        'drpai_freq',
        default_value='2',
        description='DRP-AI frequency factor (1-127). 1-2 = 1 GHz, 3 = 630 MHz, ...'
    )

    declare_model_dir = DeclareLaunchArgument(
        'model_dir',
        default_value='/hand_gesture_drp_ros/hand_yolov3_onnx',
        description='Path to the DRP-AI compiled model directory'
    )

    declare_label_list = DeclareLaunchArgument(
        'label_list',
        default_value='/hand_gesture_drp_ros/labels.txt',
        description='Path to the gesture label list file'
    )

    declare_camera_device = DeclareLaunchArgument(
        'camera_device',
        default_value='',
        description='V4L2 device path (e.g. /dev/video2). Leave empty for auto-detect.'
    )

    # ── Node ─────────────────────────────────────────────────────────────────
    gesture_node = Node(
        package='hand_gesture_drp_ros',
        executable='hand_gesture_recognition_node',
        name='hand_gesture_recognition',
        output='screen',
        emulate_tty=True,
        parameters=[{
            'drpai_freq':      LaunchConfiguration('drpai_freq'),
            'model_dir':       LaunchConfiguration('model_dir'),
            'label_list':      LaunchConfiguration('label_list'),
            'camera_device':   LaunchConfiguration('camera_device'),
            'image_topic':     '/hand_gesture/image_raw',
            'detection_topic': '/hand_gesture/detection',
            'timing_topic':    '/hand_gesture/timing',
        }],
    )

    return LaunchDescription([
        declare_drpai_freq,
        declare_model_dir,
        declare_label_list,
        declare_camera_device,
        gesture_node,
    ])
