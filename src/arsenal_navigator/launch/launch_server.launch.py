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
    

    robot_description_file = os.path.join(pkg_path, 'description', 'arsenal_robot_description.urdf.xacro')
    robot_description = Command(['xacro ', robot_description_file])

    robot_state_publisher_path = os.path.join(pkg_path, 'launch', 'rsp.launch.py')
    RobotStatePublisher = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(robot_state_publisher_path),
        launch_arguments={'robot_description': robot_description}.items()
    )

    twist_mux_parameters = os.path.join(pkg_path, 'config', 'twist_mux.yaml')
    TwistMux = Node(
        package="twist_mux",
        executable="twist_mux",
        parameters=[twist_mux_parameters, {'use_sim_time': False}],
        remappings=[('/cmd_vel_out', '/diff_cont/cmd_vel_unstamped')]
    )



    controller_parameters = os.path.join(get_package_share_directory(package_name),'config','controllers.yaml')

    ControllerManager = Node(
        package="controller_manager",
        executable="ros2_control_node",
        parameters=[{'robot_description': robot_description},
                    controller_parameters]
    )

    DelayedControllerManager = TimerAction(period=3.0, actions=[ControllerManager])

    DifferentialDriver = Node(
        package="controller_manager",
        executable="spawner",
        arguments=["diff_cont"],
    )

    DelayedDifferntialDriver = RegisterEventHandler(
        event_handler=OnProcessStart(
            target_action=ControllerManager,
            on_start=[DifferentialDriver],
        )
    )

    JointBroadcaster = Node(
        package="controller_manager",
        executable="spawner",
        arguments=["joint_broad"],
    )

    DelayedJointBroadcaster = RegisterEventHandler(
        event_handler=OnProcessStart(
            target_action=ControllerManager,
            on_start=[JointBroadcaster],
        )
    )

    return LaunchDescription([
        RobotStatePublisher,
        TwistMux,
        DelayedControllerManager,
        DelayedDifferntialDriver,
        DelayedJointBroadcaster

    ])
