#!/bin/bash
cd "$(dirname "$0")"  # cd into same direcetory as the bash script

# Check if at least one argument is provided
if [ "$#" -lt 1 ]; then
    echo "Usage: $0 <launch_file1> [<launch_file2> ... <launch_fileN>]"
    exit 1
fi

# Source ROS2
source /opt/ros/humble/setup.bash

# Source the ROS2 setup file from the custom workspace
source install/setup.bash


# Iterate over each provided launch file and run it
for launch_file in "$@"
do
    echo "Running launch file: $launch_file"
    ros2 launch arsenal_navigator "$launch_file" > "$launch_file.log" 2>&1 &
done

