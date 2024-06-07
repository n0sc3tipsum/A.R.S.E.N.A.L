import os

from ament_index_python.packages import get_package_share_directory


from launch import LaunchDescription
from launch.actions import IncludeLaunchDescription
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.actions import RegisterEventHandler, TimerAction
from launch.event_handlers import OnProcessExit, OnProcessStart
from launch_ros.actions import Node
from launch.substitutions import LaunchConfiguration, Command
from launch.actions import DeclareLaunchArgument



def generate_launch_description():

    package_name='arsenal_navigator' 
    pkg_path = os.path.join(get_package_share_directory('arsenal_navigator'))
    
    twist_mux_params = os.path.join(get_package_share_directory(package_name),'config','twist_mux.yaml')
    rviz_config_path = os.path.join(pkg_path, 'config/sim_bot.rviz')
    robot_description = Command(['ros2 param get --hide-type /robot_state_publisher robot_description'])

    rsp = IncludeLaunchDescription(
                PythonLaunchDescriptionSource([os.path.join(
                    get_package_share_directory(package_name),'launch','rsp.launch.py'
                )]), 
                launch_arguments={'use_sim_time': 'false', 'use_ros2_control': 'false'}.items()
    )

    odom_calculator = Node(
            package="arsenal_navigator",
            executable="OdomCalculator"
    )

    twist_mux = Node(
            package="twist_mux",
            executable="twist_mux",
            parameters=[twist_mux_params, {'use_sim_time': True}],
            remappings=[('/cmd_vel_out','/diff_cont/cmd_vel_unstamped')]
    )

    ekf = IncludeLaunchDescription(
            PythonLaunchDescriptionSource([os.path.join(
                        get_package_share_directory(package_name),'launch','ekf.launch.py'
                    )])
    )

    # Launch them all
    return LaunchDescription([
        rsp,
        twist_mux,
        odom_calculator,
        ekf
    ])
