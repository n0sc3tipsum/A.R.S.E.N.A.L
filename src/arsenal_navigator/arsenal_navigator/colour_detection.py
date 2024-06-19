import rclpy
from rclpy.node import Node
from sensor_msgs.msg import Image
from cv_bridge import CvBridge
import cv2
import numpy as np

class ColorDetectionNode(Node):
    def __init__(self):
        super().__init__('color_detection_node')
        
        # Declare parameters
        self.declare_parameter('colour_chosen', 'blue')
        
        # Get the target color
        self.colour_chosen = self.get_parameter('colour_chosen').get_parameter_value().string_value
        
        # Subscribe to the camera image topic
        self.subscription = self.create_subscription(
            Image,
            '/camera/image_raw',
            self.listener_callback,
            10)
        
        self.bridge = CvBridge()
        
        self.get_logger().info(f'Starting colour detection for {self.colour_chosen} color')

    def listener_callback(self, msg):
        
        cv_image = self.bridge.imgmsg_to_cv2(msg, 'bgr8')

        
        hsv_image = cv2.cvtColor(cv_image, cv2.COLOR_BGR2HSV)

        
        if self.colour_chosen == 'blue':
            lower_bound = np.array([110, 160, 0])
            upper_bound = np.array([145, 250, 250])
        elif self.colour_chosen == 'red':
            lower_bound = np.array([0, 120, 70])
            upper_bound = np.array([10, 255, 255])
        elif self.colour_chosen == 'green':
            lower_bound = np.array([40, 100, 50])
            upper_bound = np.array([90, 255, 255])
        else:
            self.get_logger().info(f'Color {self.colour_chosen} not recognized. Defaulting to blue.')
            lower_bound = np.array([100, 150, 0])
            upper_bound = np.array([140, 255, 255])

        
        mask = cv2.inRange(hsv_image, lower_bound, upper_bound)

        
        contours, _ = cv2.findContours(mask, cv2.RETR_TREE, cv2.CHAIN_APPROX_SIMPLE)

        
        for contour in contours:
            x, y, w, h = cv2.boundingRect(contour)
            cv2.rectangle(cv_image, (x, y), (x+w, y+h), (0, 255, 0), 2)

        
        cv2.imshow('Colour_Detection', cv_image)
        cv2.waitKey(1)

def main(args=None):
    rclpy.init(args=args)
    node = ColorDetectionNode()
    rclpy.spin(node)
    node.destroy_node()
    rclpy.shutdown()
    cv2.destroyAllWindows()

if __name__ == '__main__':
    main()

