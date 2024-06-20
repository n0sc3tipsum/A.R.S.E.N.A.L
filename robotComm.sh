#!/bin/bash

# Define the target machine and user
SENSOR=$1         #  PiCam = 0, anything else = LiDAR
TARGET_USER=$2
TARGET_MACHINE=$3 # IP address or hostname

# Formulate the commands to run on the remote machine
REMOTE_COMMANDS=""

if [ "$SENSOR" -eq 0 ]; then
  REMOTE_COMMANDS="source /opt/ros/humble/setup.bash; ros2 run v4l2_camera v4l2_camera_node --ros-args -p image_size:='[640,480]' > 'picam.log' 2>&1 &"
else
  REMOTE_COMMANDS="ros2 launch rplidar_ros rplidar_a1_launch.py &"
fi

# Perform SSH and run the remote commands
ssh $TARGET_USER@$TARGET_MACHINE "$REMOTE_COMMANDS"

