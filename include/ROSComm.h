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
#include <std_msgs/msg/float64.h>
#include <std_msgs/msg/int32.h>

#include "rosidl_runtime_c/string_functions.h"
#include "rosidl_runtime_c/primitives_sequence_functions.h"

#include "micro_ros_utilities/type_utilities.h"
#include "micro_ros_utilities/string_utilities.h"
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
#define NULLCHECK(ptr_in,msg) {if (ptr_in== NULL) {Serial.print("ERROR NULL: "); Serial.println(msg);} else{Serial.print("SUCCESS : "); Serial.println(msg);}}

typedef struct
{

    sensors_event_t accel;
    sensors_event_t gyro;
    sensors_event_t temp;

}imu_data_t;

typedef struct
{
    float left_speed;
    float left_pos;
    float right_speed;
    float right_pos;
}motor_data_t;

class ROSComm
{
public:

    ROSComm();

    void Init(IPAddress agent__ip = IPAddress(191, 165, 28,0), size_t agent_port = 0);
    void CommandCallback(const void *cmd_vel_recv, float *angular_setpoint, float *linear_setpoint);
    void PublishCallback(motor_data_t *motor_data, imu_data_t *imu_data, int BattLevel, int BattPower);
    void CreatePublishers();
    void CreateSubscribers();
    void CreateMessages();
    void Cleanup();
    void InitMessages();
    void getData(motor_data_t *motor_data, imu_data_t *imu_data, int BattLevel, int BattPower);

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
    
    //rcl_publisher_t _left_wheel_state_pub;
    //rcl_publisher_t _right_wheel_state_pub;
    rcl_publisher_t _joint_state_pub;
    rcl_publisher_t _imu_pub;
    rcl_publisher_t _batt_lvl_pub;
    rcl_publisher_t _batt_pwr_pub;


    rcl_subscription_t          _cmd_vel_sub;
    geometry_msgs__msg__Twist   _cmd_vel_msg;
 
    builtin_interfaces__msg__Time   _time_stamp;
    sensor_msgs__msg__Imu           _imu_msg;
    //sensor_msgs__msg__JointState    _lwheel_state_msg;
    //sensor_msgs__msg__JointState    _rwheel_state_msg;
    std_msgs__msg__Int32            _battery_pwr_msg;
    std_msgs__msg__Int32            _battery_lvl_msg;
    sensor_msgs__msg__JointState    _joint_states_msg;
    micro_ros_utilities_memory_conf_t _msg_conf;
};

#endif