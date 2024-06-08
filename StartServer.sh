#!/bin/bash

# Exit on first error
set -e
PACKAGE_NAME="arsenal_navigator"
CUSHION=3
# Source the ROS2 installation (make sure this path matches your ROS2 setup)
source /opt/ros/foxy/setup.bash

# Define a function to wait for a ROS2 node to be ready
function wait-for-node() {
    local node_name=$1
    echo "Waiting for $node_name to be ready..."
    until ros2 node list | grep -q $node_name; do
        sleep 1
    done
}

# Launch the simulation and wait for a node to be ready
echo "Launching the simulation..."
ros2 launch <PACKAGE_NAME> launch_sim.launch.py &
wait-for-node "/your_simulation_node_name"  # Replace with an actual node name from your simulation
sleep
# Launch RViz and wait for it to be ready
echo "Launching RViz..."
ros2 launch PACKAGE_NAME launchRviz.launch.py &
wait-for-node "/rviz"  # Replace with the actual RViz node name

# Launch SLAM and wait for it to be ready
echo "Launching SLAM..."
ros2 launch PACKAGE_NAME launch_slam.launch.py &
wait-for-node "/slam_node_name"  # Replace with an actual SLAM node name

# Launch localization and wait for it to be ready
echo "Launching Localization..."
ros2 launch PACKAGE_NAME launch_localisation.launch.py &
wait-for-node "/localisation_node_name"  # Replace with an actual localization node name

# Launch Navigation2 and wait for it to be ready
echo "Launching Navigation2..."
ros2 launch PACKAGE_NAME launch_nav2.launch.py &
wait-for-node "/nav2_node_name"  # Replace with an actual Navigation2 node name

echo "All packages have been launched successfully."

