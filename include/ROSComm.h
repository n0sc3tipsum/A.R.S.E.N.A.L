#ifndef ROSCOMM_H
#define ROSCOMM_H

#include <micro_ros_platformio.h>

#include <rcl/rcl.h>
#include <rcl/node.h>
#include <rclc/rclc.h>
#include <rclc/executor.h>

#include <geometry_msgs/msg/twist.h>
#include <geometry_msgs/msg/vector3.h>
#include <sensor_msgs/msg/imu.h>
#include <sensor_msgs/msg/battery_state.h>
#include <sensor_msgs/msg/joint_state.h>

#include "rosidl_runtime_c/string_functions.h"
#include "rosidl_runtime_c/primitives_sequence_functions.h"

#include "builtin_interfaces/msg/time.h"
#include "builtin_interfaces/msg/detail/time__functions.h"

#include "step.h"
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <IPAddress.h>

#include <stdio.h>
#include <cmath>
#include <cstring>

#define RCSOFTCHECK(fn, er) { rcl_ret_t temp_rc = fn; if((temp_rc != RCL_RET_OK)){Serial.print("ERROR : "); Serial.print(er); Serial.print(" --- Code : "); Serial.println(temp_rc);} else{Serial.print("SUCCESS : "); Serial.println(er);}}


class ROSComm
{
public:

    ROSComm();

    void Init(IPAddress agent__ip = IPAddress(191, 165, 28,0), size_t agent_port = 0);
    void CommandCallback(const void *cmd_vel_recv);
    void PublishCallback(step *lmotor, step *rmotor, Adafruit_MPU6050 *imu);
    void CreatePublishers();
    void CreateSubscribers();
    void CreateMessages(sensor_msgs__msg__Imu &imu_data, sensor_msgs__msg__JointState &left_wheel_state, sensor_msgs__msg__JointState &right_wheel_state);
    void getData(sensor_msgs__msg__Imu &imu_data, sensor_msgs__msg__JointState &left_wheel_state, sensor_msgs__msg__JointState &right_wheel_state, 
                 step *lmotor, step *rmotor, Adafruit_MPU6050 *imu);

    rosidl_runtime_c__String getFrameId(const char *input);
    
    IPAddress _esp_ip;
    IPAddress _agent_ip;
    size_t    _agent_port;
    char      *_ssid;
    char      *_pswd;

    
    rcl_node_t       node;
    rcl_allocator_t  allocator;
    rclc_executor_t  executor;
    rclc_support_t   support;
    rcl_timer_t      timer;
    
    rcl_publisher_t _left_wheel_state_pub;
    rcl_publisher_t _right_wheel_state_pub;

    rcl_publisher_t _imu_pub;
    rcl_publisher_t _batt_state_pub;

    rcl_subscription_t          _cmd_vel_sub;
    geometry_msgs__msg__Twist   _cmd_vel_msg;
 
    builtin_interfaces__msg__Time   _time_stamp;
    sensor_msgs__msg__Imu           _imu_msg;
    sensor_msgs__msg__JointState    _lwheel_state_msg;
    sensor_msgs__msg__JointState    _rwheel_state_msg;
};

#endif