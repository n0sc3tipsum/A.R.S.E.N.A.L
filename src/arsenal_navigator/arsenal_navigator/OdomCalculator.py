import rclpy
from rclpy.node import Node
from std_msgs.msg import Float32  # Assuming input is Float32 for radians
from geometry_msgs.msg import Quaternion
from nav_msgs.msg import Odometry
from sensor_msgs.msg import JointState
import tf_transformations
import math
import signal

class OdomCalculator(Node):
    def __init__(self):
        super().__init__('odometry_calculator')
        self.wheelStateSub = self.create_subscription(
            JointState, 
            'esp/wheel_states', 
            self.wheel_state_callback, 10)
   
        self.odom_pub = self.create_publisher(Odometry, 'arsenalServer/odom', 10)
        self.get_logger().info('Subscriptions and publisher created successfully.')
        
        self.last_wheel_left = 0.0
        self.last_wheel_right = 0.0
        self.delta_rad_left = 0.0
        self.delta_rad_right = 0.0
        self.last_vel_left = 0.0
        self.last_vel_right = 0.0
        self.x = 0.0
        self.y = 0.0
        self.th = 0.0
        self.wheel_radius = 0.1  # radius of the wheel
        self.wheel_base = 0.5    # distance between the wheels
        self.last_time = self.get_clock().now()
        self.get_logger().info('Odometry Calculator Node started.')

    def wheel_state_callback(self, msg):
        try:
            # Get the indices of left and right wheel joints
            left_wheel_index = msg.name.index("left_wheel_joint")
            right_wheel_index = msg.name.index("right_wheel_joint")
            
            # Extract the positions and velocities
            left_wheel_position = msg.position[left_wheel_index]
            right_wheel_position = msg.position[right_wheel_index]
            left_wheel_velocity = msg.velocity[left_wheel_index]
            right_wheel_velocity = msg.velocity[right_wheel_index]
            
            self.get_logger().debug(f'Received left wheel state: position={left_wheel_position}, velocity={left_wheel_velocity}')
            self.get_logger().debug(f'Received right wheel state: position={right_wheel_position}, velocity={right_wheel_velocity}')
            
            # Calculate deltas
            self.delta_rad_left = left_wheel_position - self.last_wheel_left
            self.delta_rad_right = right_wheel_position - self.last_wheel_right
            
            # Update last positions and velocities
            self.last_wheel_left = left_wheel_position
            self.last_wheel_right = right_wheel_position
            self.last_vel_left = left_wheel_velocity
            self.last_vel_right = right_wheel_velocity
            
            self.calculate_odometry()
        except ValueError as e:
            self.get_logger().error(f'Error in wheel state callback: {e}')

    def calculate_odometry(self):
        current_time = self.get_clock().now()
        dt = (current_time - self.last_time).nanoseconds / 1e9
        self.last_time = current_time

        # Calculate the distance each wheel has traveled since the last measurement
        d_left = self.delta_rad_left * self.wheel_radius
        d_right = self.delta_rad_right * self.wheel_radius

        # Calculate the average distance traveled
        d = (d_right + d_left) / 2.0
        
        # Calculate the change in orientation
        delta_th = (d_right - d_left) / self.wheel_base
        
        # Calculate new position in odometry frame
        dx = d * math.cos(self.th + delta_th / 2.0)
        dy = d * math.sin(self.th + delta_th / 2.0)
        
        # Update the pose estimate
        self.x += dx
        self.y += dy
        self.th += delta_th

        # Create new odometry message
        odom_msg = Odometry()
        odom_msg.header.stamp = current_time.to_msg()
        odom_msg.header.frame_id = 'odom'
        odom_msg.child_frame_id = 'base_link'
        
        # Set the position
        odom_msg.pose.pose.position.x = self.x
        odom_msg.pose.pose.position.y = self.y
        q = tf_transformations.quaternion_from_euler(0, 0, self.th)
        odom_msg.pose.pose.orientation = Quaternion(x=q[0], y=q[1], z=q[2], w=q[3])
        
        # Set the velocity
        odom_msg.twist.twist.linear.x = d / dt
        odom_msg.twist.twist.angular.z = delta_th / dt

        # Publish the message
        self.odom_pub.publish(odom_msg)

def main(args=None):
    rclpy.init(args=args)
    odometry_calculator = OdomCalculator()

    # Handle signal interrupt (CTRL+C)
    def signal_handler(sig, frame):
        odometry_calculator.get_logger().info('Shutting down gracefully...')
        rclpy.shutdown()

    signal.signal(signal.SIGINT, signal_handler)

    rclpy.spin(odometry_calculator)
    odometry_calculator.destroy_node()
    rclpy.shutdown()

if __name__ == '__main__':
    main()
