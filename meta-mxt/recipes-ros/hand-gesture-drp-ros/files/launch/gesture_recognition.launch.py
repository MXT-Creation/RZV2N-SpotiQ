"""
Launch file for Hand Gesture Recognition ROS2 node.

Usage:
    ros2 launch hand_gesture_recognition gesture_recognition.launch.py
    ros2 launch hand_gesture_recognition gesture_recognition.launch.py drpai_freq:=3
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

    declare_frame_id = DeclareLaunchArgument(
        'frame_id',
        default_value='camera',
        description='TF frame id to stamp published images with'
    )

    # ── Node ─────────────────────────────────────────────────────────────────
    gesture_node = Node(
        package='hand_gesture_recognition',
        executable='hand_gesture_recognition_node',
        name='hand_gesture_recognition',
        output='screen',
        emulate_tty=True,
        parameters=[{
            'input_source':      'USB',
            'drpai_freq':        LaunchConfiguration('drpai_freq'),
            'model_dir':         LaunchConfiguration('model_dir'),
            'label_list':        LaunchConfiguration('label_list'),
            'image_topic':       '/hand_gesture/image_raw',
            'detection_topic':   '/hand_gesture/detection',
            'timing_topic':      '/hand_gesture/timing',
            'frame_id':          LaunchConfiguration('frame_id'),
        }],
        # Remappings if needed by downstream nodes
        remappings=[
            # ('/hand_gesture/image_raw', '/camera/image_raw'),
        ],
    )

    return LaunchDescription([
        declare_drpai_freq,
        declare_model_dir,
        declare_label_list,
        declare_frame_id,
        gesture_node,
    ])
