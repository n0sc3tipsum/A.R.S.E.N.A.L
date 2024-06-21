import os

from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import IncludeLaunchDescription
from launch.launch_description_sources import PythonLaunchDescriptionSource

def generate_launch_description():
    pkg_name = 'arsenal_navigator'
    
    try:
        pkg_path = get_package_share_directory('arsenal_navigator')
    except Exception as e:
        raise RuntimeError(f"Error getting package share directory: {e}")
    
    sim_launch_path = os.path.join(pkg_path, 'launch', 'launch_sim.launch.py')
    slam_launch_path = os.path.join(pkg_path, 'launch', 'online_async.launch.py')
    
    if not os.path.isfile(sim_launch_path):
        raise FileNotFoundError(f"Sim launch file not found: {sim_launch_path}")
    if not os.path.isfile(slam_launch_path):
        raise FileNotFoundError(f"Slam launch file not found: {slam_launch_path}")
    
    simLaunch = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(sim_launch_path)
    )

    slamLaunch = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(slam_launch_path)
    )

    return LaunchDescription([
        simLaunch,
        slamLaunch
    ])
