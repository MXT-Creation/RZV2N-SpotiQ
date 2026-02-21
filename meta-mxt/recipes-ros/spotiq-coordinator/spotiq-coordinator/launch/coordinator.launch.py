"""
SpotiQ Coordinator — launch file
Usage:
  ros2 launch spotiq_coordinator coordinator.launch.py
  ros2 launch spotiq_coordinator coordinator.launch.py debug_mode:=true
  ros2 launch spotiq_coordinator coordinator.launch.py movement_speed:=30
  ros2 launch spotiq_coordinator coordinator.launch.py debug_mode:=true movement_speed:=30
"""

from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node


def generate_launch_description():
    return LaunchDescription([
        DeclareLaunchArgument("detection_min_prob", default_value="0.35"),
        DeclareLaunchArgument("debug_mode", default_value="false",
                             description="Skip startup, go directly to SCAN for testing"),
        DeclareLaunchArgument("debug_pick", default_value="false",
                             description="Enable picking in debug mode (requires debug_mode:=true)"),
        DeclareLaunchArgument("movement_speed", default_value="50",
                             description="Robot movement speed in mm/s (default: 50)"),

        Node(
            package="spotiq_coordinator",
            executable="coordinator",
            name="spotiq_coordinator",
            output="screen",
            parameters=[{
                "detection_min_prob": LaunchConfiguration("detection_min_prob"),
                "debug_mode": LaunchConfiguration("debug_mode"),
                "debug_pick": LaunchConfiguration("debug_pick"),
                "movement_speed": LaunchConfiguration("movement_speed"),
            }],
        ),
    ])
