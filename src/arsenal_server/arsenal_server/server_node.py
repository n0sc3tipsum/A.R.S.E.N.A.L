import rclpy
import subprocess
from rclpy.node import Node
from flask import Flask, jsonify #, Response
from std_msgs.msg import String
from sensor_msgs.msg import Image
#import cv2
#import numpy as np

app = Flask(__name__)

# target velocities for left and right wheels
target_velocity_L = 0.0
target_velocity_R = 0.0

# whether or not the robot should be navigating autonomously
auto_navigate = False

# mostly recently read values of power info
powerConsumption = '---'
remainingCharge = '---'
batteryVoltage = '---'

@app.route('/')
def index():
    return 'MoolyFTW'

@app.route('/readPowerConsumption')
def readPowerConsumption():
    global powerConsumption
    return powerConsumption

@app.route('/readRemainingCharge')
def readRemainingCharge():
    global remainingCharge
    return remainingCharge

@app.route('/readBatteryVoltage')
def readBatteryVoltage():
    global batteryVoltage
    return batteryVoltage
    
#------------------------------------- 
    
@app.route('/moveForward')
def moveForward():
    global target_velocity_L, target_velocity_R
    target_velocity_L = 5.0
    target_velocity_R = 5.0
    publish_velocity()
    return 'Moving forward'

@app.route('/moveBackward')
def moveBackward():
    global target_velocity_L, target_velocity_R
    target_velocity_L = -5.0
    target_velocity_R = -5.0
    publish_velocity()
    return 'Moving backward'

@app.route('/turnLeft')
def turnLeft():
    global target_velocity_L, target_velocity_R
    target_velocity_L = -5.0
    target_velocity_R = 5.0
    publish_velocity()
    return 'Turning left'

@app.route('/turnRight')
def turnRight():
    global target_velocity_L, target_velocity_Rs
    target_velocity_L = 5.0
    target_velocity_R = -5.0
    publish_velocity()
    return 'Turning right'

@app.route('/toggleAutonomousNavigation')
def toggleAutonomousNavigation():
    global auto_navigate
    auto_navigate = not auto_navigate
    return 'Switching to autonomous navigation' if auto_navigate else 'Switching to manual navigation'
    
#-------------------------------------

@app.route('/launchServer')
def launchServer():
    return launchScript("launch_server.launch.py")

@app.route('/launchSim')
def launchSim():
    return launchScript("launchSim.sh")

@app.route('/launchMapping')
def launchMapping():
    return launchScript("launchMapping.sh")

@app.route('/launchNavigating')
def launchNavigating():
    return "launchNavigating IS NOT YET IMPLEMENTED"
    
# launchFileList = a string with the file name of each launch file, separated by spaces (e.g., "launchfile1.launch.py launchfile2.launch.py launchfile3.launch.py")
def launchScript(launchFileList):
    try:
        subprocess_result = subprocess.run(["~/arsenal_server_2/subsytemLauncher.sh " + launchFileList], shell=True, check=True, capture_output=True, text=True)
        return jsonify(message="Launched " + launchFileList, output=subprocess_result.stdout)
    except subprocess.CalledProcessError as e:
        return jsonify(message="Failed to execute " + launchFileList, error_output=e.stderr)
	
#-------------------------------------

class FlaskServerNode(Node):
    def __init__(self):
    	# call parent class constructor
        super().__init__('arsenal_server_node')
        
        # create publisher to publish command velocities
        self.publisher = self.create_publisher(String, 'cmd_vel', 10)
        
        # create subscription to bot status from raspi
        self.bot_status_subscription = self.create_subscription(String, 'bot_status', self.listener_callback_bot_status, 10) # get bot status from rasp, with a buffer of 10
        self.bot_status_subscription
             
        self.get_logger().info('Flask server node has been started.')

    def publish_velocity(self):
        msg = String()
        msg.data = str(target_velocity_L) + ', ' + str(target_velocity_R)
        self.publisher.publish(msg)
        self.get_logger().info('Published velocities: ' + msg.data)
        
    def listener_callback_bot_status(self, msg):
        global powerConsumption, remainingCharge, batteryVoltage
        powerConsumption, remainingCharge, batteryVoltage = msg.data.split(', ')
        self.get_logger().info('Subscription yielded: ' + msg.data)
        
def publish_velocity():
    node.publish_velocity()

def main(args=None):
    global node
    rclpy.init(args=args)
    node = FlaskServerNode()

    # start server in separate thread to avoid blocking with ros spin 
    import threading
    flask_thread = threading.Thread(target=app.run, kwargs={'debug': True, 'host': '0.0.0.0', 'use_reloader': False})
    flask_thread.start()

    rclpy.spin(node)
    flask_thread.join()

    rclpy.shutdown()

if __name__ == '__main__':
    main()
    
    
 # TO BUILD/RUN; run the below in dev-ws directory
 #  clear; colcon build --packages-select arsenal_server; source install/setup.bash; ros2 run arsenal_server server_node
 
 
 # FOR REFERENCE, subsystemLauncher.sh:

# #!/bin/bash
# 
# # Check if at least one argument is provided
# if [ "$#" -lt 1 ]; then
#     echo "Usage: $0 <launch_file1> [<launch_file2> ... <launch_fileN>]"
#     exit 1
# fi
#
# # Source ROS2
# source /opt/ros/humble/setup.bash
# 
# # Source the ROS2 setup file from the custom workspace
# source install/setup.bash
# 
# # Iterate over each provided launch file and run it
# for launch_file in "$@"
# do
#     echo "Running launch file: $launch_file"
#     ros2 launch arsenal_navigator "$launch_file"
# done



