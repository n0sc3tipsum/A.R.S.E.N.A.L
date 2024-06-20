import os
import rclpy
import subprocess
from rclpy.node import Node
from flask import Flask, jsonify, request, Response
from std_msgs.msg import String
from geometry_msgs.msg import Twist
from sensor_msgs.msg import Image

#import cv2
#import numpy as np



app = Flask(__name__)

# target velocities for left and right wheels
lin_x = 0.0
lin_y = 0.0
lin_z = 0.0
ang_x = 0.0
ang_y = 0.0
ang_z = 0.0

ANG_VEL = 0.5

# whether or not the robot should be navigating autonomously
auto_navigate = False

# mostly recently read values of power info
powerConsumption = '---'
remainingCharge = '---'
batteryVoltage = '---'

@app.route('/')
def index():
    return 'MoolyFTW'

@app.after_request
def add_security_headers(response: Response) -> Response:
    csp_policy = (
        "default-src 'self'; "
        "script-src 'self' http://192.168.230.66; "
        "style-src 'self'; "
        "img-src 'self'; "
        "font-src 'self'; "
        "object-src 'none'; "
        "frame-ancestors 'none'; "
        "base-uri 'self'; "
        "form-action 'self'"
    )
    response.headers['Content-Security-Policy'] = csp_policy
    return response


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
    global lin_x, lin_y, lin_z, ang_x, ang_y, ang_z
    lin_x = 3.0
    lin_y = 0.0
    lin_z = 0.0
    ang_x = 0.0
    ang_y = 0.0
    ang_z = 0.0
    publish_velocity()
    return 'Moving forward'
    
@app.route('/moveForwardAndLeft')
def moveForwardAndLeft():
    global lin_x, lin_y, lin_z, ang_x, ang_y, ang_z
    lin_x = 5.0
    lin_y = 0.0
    lin_z = 0.0
    ang_x = 0.0
    ang_y = 0.0
    ang_z = -ANG_VEL
    publish_velocity()
    return 'Moving forward and left'
    
@app.route('/moveForwardAndRight')
def moveForwardAndRight():
    global lin_x, lin_y, lin_z, ang_x, ang_y, ang_z
    lin_x = 5.0
    lin_y = 0.0
    lin_z = 0.0
    ang_x = 0.0
    ang_y = 0.0
    ang_z = ANG_VEL
    publish_velocity()
    return 'Moving forward and right'

@app.route('/moveBackward')
def moveBackward():
    global lin_x, lin_y, lin_z, ang_x, ang_y, ang_z
    lin_x = -3.0
    lin_y = 0.0
    lin_z = 0.0
    ang_x = 0.0
    ang_y = 0.0
    ang_z = 0.0
    publish_velocity()
    return 'Moving backward'
    
@app.route('/moveBackwardAndLeft')
def moveBackwardAndLeft():
    global lin_x, lin_y, lin_z, ang_x, ang_y, ang_z
    lin_x = -5.0
    lin_y = 0.0
    lin_z = 0.0
    ang_x = 0.0
    ang_y = 0.0
    ang_z = ANG_VEL
    publish_velocity()
    return 'Moving backward and left'
    
@app.route('/moveBackwardAndRight')    
def moveBackwardAndRight():
    global lin_x, lin_y, lin_z, ang_x, ang_y, ang_z
    lin_x = -5.0
    lin_y = 0.0
    lin_z = 0.0
    ang_x = 0.0
    ang_y = 0.0
    ang_z = -ANG_VEL
    publish_velocity()
    return 'Moving backward and right'

@app.route('/turnLeft') # A
def turnLeft():
    global lin_x, lin_y, lin_z, ang_x, ang_y, ang_z
    lin_x = 0.0
    lin_y = 0.0
    lin_z = 0.0
    ang_x = 0.0
    ang_y = 0.0
    ang_z = -ANG_VEL
    publish_velocity()
    return 'Turning left'

@app.route('/turnRight')
def turnRight():
    global lin_x, lin_y, lin_z, ang_x, ang_y, ang_z
    lin_x = 0.0
    lin_y = 0.0
    lin_z = 0.0
    ang_x = 0.0
    ang_y = 0.0
    ang_z = ANG_VEL
    publish_velocity()
    return 'Turning right'
    
@app.route('/haltMovement')
def haltMovement():
    global lin_x, lin_y, lin_z, ang_x, ang_y, ang_z
    lin_x = 0.0
    lin_y = 0.0
    lin_z = 0.0
    ang_x = 0.0
    ang_y = 0.0
    ang_z = 0.0
    publish_velocity()
    return 'Halting movement'

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
    return launchScript("")

@app.route('/launchMapping')
def launchMapping():
    return launchScript("online_sync.launch.py")

@app.route('/launchNavigating')
def launchNavigating():
    return launchScript('\"nav.launch.py use_sim_time:=false map_subscribe_transient_local:=true\"')
   
@app.route('/startLocalization')
def startLocalization():
    return launchScript('\"localisation.launch.py use_sim_time:=false map:=./src/arsenal_navigator/maps/demo_map2.yaml\"')
    
# launchFileList = a string with the file name of each launch file, sepsarated by spaces (e.g., "launchfile1.launch.py launchfile2.launch.py launchfile3.launch.py")
def launchScript(launchFileList):
    try:
        subprocess_result = subprocess.run(["~/arsenal_server_2/subsystemLauncher.sh " + launchFileList], shell=True, check=True, capture_output=True, text=True)
        return jsonify(message="Launched " + launchFileList, output=subprocess_result.stdout)
    except subprocess.CalledProcessError as e:
        return jsonify(message="Failed to execute " + launchFileList, error_output=e.stderr)
	
#-------------------------------------

@app.route('/startPiCam', methods=['POST'])
def startPiCam():
    raspi_ip = request.get_data(as_text=True)
    print("STARTING PiCam - RECEIVED RASPI IP: " + raspi_ip)
    try:
        subprocess_result = subprocess.run(["~/dev_ws/robotComm.sh 0 n0sc3tipsum " + raspi_ip], shell=True, check=True, capture_output=True, text=True)
        return jsonify(message="PiCam started", output=subprocess_result.stdout)
    except subprocess.CalledProcessError as e:
        return jsonify(message="Failed to start PiCam", error_output=e.stderr)

@app.route('/startLiDAR', methods=['POST'])
def startLiDAR():
    raspi_ip = request.get_data(as_text=True)
    print("STARTING LiDAR - RECEIVED RASPI IP: " + raspi_ip)
    try:
        subprocess_result = subprocess.run(["~/dev_ws/robotComm.sh 1 n0sc3tipsum " + raspi_ip], shell=True, check=True, capture_output=True, text=True)
        return jsonify(message="LiDAR started", output=subprocess_result.stdout)
    except subprocess.CalledProcessError as e:
        return jsonify(message="Failed to start LiDAR", error_output=e.stderr)

#-------------------------------------

@app.route('/startFoxgloveBridge')
def startFoxgloveBridge():
    print("STARTING FOXGLOVE BRIDGE")
    try:
        subprocess_result = subprocess.run(["~/dev_ws/startFoxgloveBridge.sh"], shell=True, check=True, capture_output=True, text=True)
        return jsonify(message="Started Foxglove Bridge", output=subprocess_result.stdout)
    except subprocess.CalledProcessError as e:
        return jsonify(message="Failed to start Foxglove Bridge", error_output=e.stderr)

#-------------------------------------

class FlaskServerNode(Node):
    def __init__(self):
    	# call parent class constructor
        super().__init__('arsenal_server_node')
        
        # create publisher to publish command velocities
        self.publisher = self.create_publisher(Twist, 'manual_cmd_vel', 10)
        
        # create subscription to bot status from raspi
        self.bot_status_subscription = self.create_subscription(String, 'bot_status', self.listener_callback_bot_status, 10) # get bot status from rasp, with a buffer of 10
        self.bot_status_subscription
             
        self.get_logger().info('Flask server node has been started.')

    def publish_velocity(self):
        msg = Twist()
        
        msg.linear.x  = lin_x
        msg.linear.y  = lin_y
        msg.linear.z  = lin_z
        msg.angular.x = ang_x
        msg.angular.y = ang_y
        msg.angular.z = ang_z
        
        self.publisher.publish(msg)
        self.get_logger().info('Published velocities')
        
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




