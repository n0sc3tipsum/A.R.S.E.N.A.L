import rclpy
from rclpy.node import Node
from std_msgs.msg import Float32  # Assuming input is Float32 for radians
from geometry_msgs.msg import Quaternion
from nav_msgs.msg import Odometry
from sensor_msgs.msg import JointState
import tf_transformations
import math

class OdometryCalculator(Node):
    def __init__(self):
        super().__init__('odometry_calculator')
        self.subscription_wheel1 = self.create_subscription(
            JointState, 
            '/esp/right_wheel_state', 
            self.listener_callback_wheel1, 10)
        self.subscription_wheel2 = self.create_subscription(
            JointState, 
            '/esp/left_wheel_state', 
            self.listener_callback_wheel2, 10)
        
        self.odom_pub = self.create_publisher(Odometry, 'arsenalServer/odom', 10)
        self.get_logger().info('Subscriptions and publisher created successfully.')
        self.last_wheel1 = 0.0
        self.last_wheel2 = 0.0
        self.delta_rad1 = 0.0
        self.delta_rad2 = 0.0
        self.last_vel1 = 0.0
        self.last_vel2 = 0.0
        self.x = 0.0
        self.y = 0.0
        self.th = 0.0
        self.wheel_radius = 0.1  # radius of the wheel
        self.wheel_base = 0.5    # distance between the wheels
        self.last_time = self.get_clock().now()
        self.get_logger().info('Odometry Calculator Node started.')



    def listener_callback_wheel1(self, msg):
        self.get_logger().debug(f'Received right wheel state: position={msg.position}, velocity={msg.velocity}')
        self.delta_rad1 = msg.position - self.last_wheel1
        self.last_wheel1 = msg.position
        self.last_vel1 = msg.velocity
        self.calculate_odometry()

    def listener_callback_wheel2(self, msg):
        self.get_logger().debug(f'Received left wheel state: position={msg.position}, velocity={msg.velocity}')
        self.delta_rad2 = msg.position - self.last_wheel2
        self.last_wheel2 = msg.position
        self.last_vel2 = msg.velocity
        self.calculate_odometry()

    def calculate_odometry(self):
        current_time = self.get_clock().now()
        dt = (current_time - self.last_time).nanoseconds / 1e9
        self.last_time = current_time

        # Calculate the distance each wheel has traveled since the last measurement
        d_left = self.delta_rad1 * self.wheel_radius
        d_right = self.delta_rad2 * self.wheel_radius

        # Calculate the average distance traveled
        d = (d_right + d_left) / 2.0
        
        # Calculate the change in orientation
        theta = math.asin((d_right - d_left) / self.wheel_base)
        
        # Calculate new position in odometry frame
        dx = d * math.cos(theta)
        dy = d * math.sin(theta)
        
        # Update the pose estimate
        self.x += dx
        self.y += dy
        self.th += theta

        avg_vel = (self.last_vel1 + self.last_vel2)/2
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
        odom_msg.twist.twist.linear.x = avg_vel
        odom_msg.twist.twist.angular.z = theta / dt

        # Publish the message
        self.odom_pub.publish(odom_msg)

def main(args=None):
    rclpy.init(args=args)
    odometry_calculator = OdometryCalculator()
    rclpy.spin(odometry_calculator)
    odometry_calculator.destroy_node()
    rclpy.shutdown()

if __name__ == '__main__':
    main()
