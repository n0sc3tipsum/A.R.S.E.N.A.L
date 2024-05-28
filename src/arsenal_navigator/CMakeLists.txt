cmake_minimum_required(VERSION 3.5)
project(arsenal_navigator)

#find depend
find_package(ament_cmake REQUIRED)
find_package(rclpy REQUIRED)
find_package(launch REQUIRED)
find_package(launch_ros REQUIRED)
find_package(xacro REQUIRED)
find_package(std_msgs REQUIRED)
find_package(robot_state_publisher REQUIRED)

#install launch fiels
install(
    DIRECTORY description launch rviz
    DESTINATION share/${PROJECT_NAME}
)

ament_package()