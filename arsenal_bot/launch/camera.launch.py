import os

from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node

def generate_launch_description():
    # Define launch configuration variables
    width = LaunchConfiguration('width', default='640')
    height = LaunchConfiguration('height', default='480')
    framerate = LaunchConfiguration('framerate', default='30')
    camera_frame_id = LaunchConfiguration('camera_frame_id', default='camera_link_optical')
    brightness = LaunchConfiguration('brightness', default='50')
    target_color = LaunchConfiguration('target_color', default='blue')

    return LaunchDescription([

        # Declare launch arguments
        DeclareLaunchArgument(
            'width',
            default_value=width,
            description='Width of the image'),
        
        DeclareLaunchArgument(
            'height',
            default_value=height,
            description='Height of the image'),
        
        DeclareLaunchArgument(
            'framerate',
            default_value=framerate,
            description='Frame rate of the camera'),
        
        DeclareLaunchArgument(
            'camera_frame_id',
            default_value=camera_frame_id,
            description='Frame ID of the camera'),
        
        DeclareLaunchArgument(
            'brightness',
            default_value=brightness,
            description='Brightness of the image'),

        DeclareLaunchArgument(
            'target_color',
            default_value=target_color,
            description='Target color for detection'),

        # Camera node configuration
        Node(
            package='raspicam_node',
            executable='raspicam_node',
            name='raspicam_node',
            parameters=[{
                'width': width,
                'height': height,
                'framerate': framerate,
                'camera_frame_id': camera_frame_id,
                'brightness': brightness
                }],
            output='screen'
        ),

        # Color detection node configuration
        Node(
            package='arsenal_bot',
            executable='color_detection',
            name='color_detection_node',
            parameters=[{
                'target_color': target_color
                }],
            output='screen'
        )
    ])
# width: Width of the camera image. Default is 640.
# height: Height of the camera image. Default is 480.
# framerate: Frame rate of the camera. Default is 30.
# camera_frame_id: Frame ID for the camera. Default is camera_link_optical.
# brightness: Brightness of the image. Default is 50
