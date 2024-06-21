import os

from ament_index_python.packages import get_package_share_directory

from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, SetEnvironmentVariable, IncludeLaunchDescription
from launch.substitutions import LaunchConfiguration, Command
from launch_ros.actions import Node
from nav2_common.launch import RewrittenYaml
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.actions import RegisterEventHandler, TimerAction
from launch.event_handlers import OnProcessExit, OnProcessStart


def generate_launch_description():
    pkg_name='arsenal_navigator' 
    pkg_path = os.path.join(get_package_share_directory(pkg_name))

    simLaunch = IncludeLaunchDescription(
        PythonLaunchDescriptionSource([os.path.join(
            get_package_share_directory(pkg_name),'launch','launch_sim.launch.py'
    )]))
    
    amclLaunch = IncludeLaunchDescription(
        PythonLaunchDescriptionSource([os.path.join(
            get_package_share_directory(pkg_name), 'launch', 'localisation.launch.py'
        )]), launch_arguments={'use_sim_time':'true'}.items()
    )

    navLaunch = IncludeLaunchDescription(
        PythonLaunchDescriptionSource([os.path.join(
            get_package_share_directory(pkg_name), 'launch', 'nav.launch.py'
        )]), launch_arguments={'use_sim_time': 'true'}.items()
    )

    return LaunchDescription([
        simLaunch,
        amclLaunch,
        navLaunch
    ])
