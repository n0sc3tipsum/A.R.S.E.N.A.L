import rclpy
from rclpy.node import Node
from std_msgs.msg import String

class testerNode(Node):
    def __init__(self):
        super().__init__('test_node')
        self.publisher_ = self.create_publisher(String, 'topic', 10)
        timer_period = 2
        self.timer = self.create_timer(timer_period, self.timer_callback)

    def timer_callback(self):
        msg = String()
        msg.data = 'testing'
        self.publisher_.publish(msg)
        self.get_logger().info('publishing : "%s"' % msg.data)

def main(args=None):
    rclpy.init(args=args)
    node = testerNode()
    rclpy.spin(node)
    node.destroy_node()
    rclpy.shutdown()

if __name__=='__main__':
    main()