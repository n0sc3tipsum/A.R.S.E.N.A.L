#include "ROSComm.h"


ROSComm::ROSComm()
{
    _agent_port = 8888;
    _ssid = "BA36 Hyperoptic 1Gbps Broadband";
    _pswd = "pkuusr5x";
    _msg_conf = {0};
 
}

void ROSComm::Init(IPAddress agent_ip, size_t agent_port)
{
    Serial.println("Setting micro-ROS wifi transports");

    set_microros_wifi_transports(_ssid, _pswd, _agent_ip, _agent_port);
    delay(2000);

    // Init with domain id = 0
    rcl_init_options_t init_options = rcl_get_zero_initialized_init_options();
    allocator = rcl_get_default_allocator();

    RCSOFTCHECK(rcl_init_options_init(
        &init_options, 
        allocator),
        "Init RCL Options");

    RCSOFTCHECK(rcl_init_options_set_domain_id(
        &init_options, 
        0), 
        "Init Domain ID 0");

    

    // Initialize rclc support object with custom options
    RCSOFTCHECK(rclc_support_init_with_options(
        &support, 
        0, 
        NULL, 
        &init_options, 
        &allocator), 
        "Init Support Object");
    //RCSOFTCHECK(rclc_support_init(&support, 0, NULL, &allocator), "Init Default SUpport");


    // Init node with configured support object
    RCSOFTCHECK(rclc_node_init_default(
        &node, 
        "esp_node", "",
        &support), 
        "Init ESP Node");

    Serial.println("");
    CreatePublishers();
    Serial.println("");
    CreateSubscribers();
    Serial.println("");
    InitMessages();

    Serial.println("Initialising Executor");
    unsigned int num_handles = 2;
    RCSOFTCHECK(rclc_executor_init(
        &executor, 
        &support.context, 
        num_handles, 
        &allocator), 
        "Init Executor");

    Serial.println("");
    Serial.println("---- Initialisation Complete ----");
    Serial.println("");

}

void ROSComm::CommandCallback(const void *cmd_vel_recv)
{
    _cmd_vel_msg = *(geometry_msgs__msg__Twist *) cmd_vel_recv;

    float linear_vel_setpoint = _cmd_vel_msg.linear.x;
    float angular_vel_setpoint = _cmd_vel_msg.angular.z;
    Serial.println("Got Cmd_Vel !");

    Serial.print("Linear Velocity Setpoint : ");
    Serial.println(linear_vel_setpoint);
    Serial.print("Angular Velocity Setpoint : ");
    Serial.println(angular_vel_setpoint);
    /*Control Integration*/
}

void ROSComm::CreatePublishers()
{
    Serial.println("Initialising Publishers...");

    _imu_pub = rcl_get_zero_initialized_publisher();
    _left_wheel_state_pub = rcl_get_zero_initialized_publisher();
    _right_wheel_state_pub = rcl_get_zero_initialized_publisher();
        

    RCSOFTCHECK(rclc_publisher_init_default(
		&_imu_pub,
		&node,
		ROSIDL_GET_MSG_TYPE_SUPPORT(sensor_msgs, msg, Imu),
		"esp/imu"), 
        "Init IMU Publisher");


    RCSOFTCHECK(rclc_publisher_init_default(
		&_batt_state_pub,
		&node,
		ROSIDL_GET_MSG_TYPE_SUPPORT(sensor_msgs, msg, BatteryState),
		"esp/BatteryState"),
        "Init Battery State Publisher");


    RCSOFTCHECK(rclc_publisher_init_default(
        &_left_wheel_state_pub,
        &node,
        ROSIDL_GET_MSG_TYPE_SUPPORT(sensor_msgs, msg, JointState),
        "esp/left_wheel_state"),
        "Init Left Wheel State Publisher");

    RCSOFTCHECK(rclc_publisher_init_default(
        &_right_wheel_state_pub,
        &node,
        ROSIDL_GET_MSG_TYPE_SUPPORT(sensor_msgs, msg, JointState),
        "esp/right_wheel_state"),
        "Init Right Wheel State Publisher");
}

void ROSComm::CreateSubscribers()
{
    Serial.println("Initializing Subscribers");

    RCSOFTCHECK(rclc_subscription_init_default(
        &_cmd_vel_sub,
        &node,
        ROSIDL_GET_MSG_TYPE_SUPPORT(geometry_msgs, msg, Twist),
        "/cmd_vel"),
        "Init cmd_vel Subscriber");
}

void ROSComm::Cleanup()
{
    RCSOFTCHECK(rclc_executor_fini(&executor), "Shutting Down Executor");
    RCSOFTCHECK(rcl_publisher_fini(&_imu_pub, &node),"Shutting Down Imu Pub");
    RCSOFTCHECK(rcl_publisher_fini(&_left_wheel_state_pub, &node),"Shutting Down Left Wheel Pub");
    RCSOFTCHECK(rcl_publisher_fini(&_right_wheel_state_pub, &node),"Shutting Down Right Wheel Pub");
    RCSOFTCHECK(rcl_publisher_fini(&_batt_state_pub, &node),"Shutting Down Batt State Pub");
    RCSOFTCHECK(rcl_subscription_fini(&_cmd_vel_sub, &node), "Shutting Down Cmd_vel Sub");
    RCSOFTCHECK(rcl_timer_fini(&timer), "Shutting Down Timer");
    RCSOFTCHECK(rcl_node_fini(&node), "Shutting Down ESP Node");
    RCSOFTCHECK(rclc_support_fini(&support), "Shutting Down Support");

    RCSOFTCHECK(!micro_ros_utilities_destroy_message_memory(
                ROSIDL_GET_MSG_TYPE_SUPPORT(sensor_msgs, msg, Imu),
                &_imu_msg, _msg_conf), 
                "De-Alloc  IMU Message Memory");

    RCSOFTCHECK(!micro_ros_utilities_destroy_message_memory(
                ROSIDL_GET_MSG_TYPE_SUPPORT(sensor_msgs, msg, JointState),
                &_lwheel_state_msg, _msg_conf), 
                "De-Alloc  Left Wheel Message Memory");

    RCSOFTCHECK(!micro_ros_utilities_destroy_message_memory(
                ROSIDL_GET_MSG_TYPE_SUPPORT(sensor_msgs, msg, JointState),
                &_rwheel_state_msg, _msg_conf), 
                "De-Alloc  Right Wheel Message Memory");

    RCSOFTCHECK(!micro_ros_utilities_destroy_message_memory(
                ROSIDL_GET_MSG_TYPE_SUPPORT(geometry_msgs, msg, Twist),
                &_cmd_vel_msg, _msg_conf), 
                "De-Alloc  CMD Vel Message Memory");

    RCSOFTCHECK(!micro_ros_utilities_destroy_message_memory(
                ROSIDL_GET_MSG_TYPE_SUPPORT(builtin_interfaces, msg, Time),
                &_time_stamp, _msg_conf), 
                "De-Alloc Time Stamp Message Memory");

}

void ROSComm::InitMessages()
{

    RCSOFTCHECK(micro_ros_utilities_create_message_memory(
                ROSIDL_GET_MSG_TYPE_SUPPORT(sensor_msgs, msg, Imu),
                &_imu_msg, _msg_conf), 
                "Alloc IMU Message Memory");

    RCSOFTCHECK(micro_ros_utilities_create_message_memory(
                ROSIDL_GET_MSG_TYPE_SUPPORT(sensor_msgs, msg, JointState),
                &_lwheel_state_msg, _msg_conf), 
                "Alloc Left Wheel Message Memory");

    RCSOFTCHECK(micro_ros_utilities_create_message_memory(
                ROSIDL_GET_MSG_TYPE_SUPPORT(sensor_msgs, msg, JointState),
                &_rwheel_state_msg, _msg_conf), 
                "Alloc Right Wheel Message Memory");

    RCSOFTCHECK(micro_ros_utilities_create_message_memory(
                ROSIDL_GET_MSG_TYPE_SUPPORT(geometry_msgs, msg, Twist),
                &_cmd_vel_msg, _msg_conf), 
                "Alloc CMD Vel Message Memory");

    RCSOFTCHECK(micro_ros_utilities_create_message_memory(
                ROSIDL_GET_MSG_TYPE_SUPPORT(builtin_interfaces, msg, Time),
                &_time_stamp, _msg_conf), 
                "Alloc Time Stamp Message Memory");


    _imu_msg.header.frame_id          = micro_ros_string_utilities_set(_imu_msg.header.frame_id, "imu_frame");
    _lwheel_state_msg.header.frame_id = micro_ros_string_utilities_set(_lwheel_state_msg.header.frame_id, "left_wheel_frame");
    _rwheel_state_msg.header.frame_id = micro_ros_string_utilities_set(_rwheel_state_msg.header.frame_id, "right_wheel_frame");

    *_lwheel_state_msg.name.data = micro_ros_string_utilities_set(*_lwheel_state_msg.name.data, "left_wheel_joint");
    *_rwheel_state_msg.name.data = micro_ros_string_utilities_set(*_rwheel_state_msg.name.data, "right_wheel_joint");
}

void ROSComm::CreateMessages()
{
    Serial.println("Creating Messages");

    static micro_ros_utilities_memory_conf_t conf = {0};


    RCSOFTCHECK(!builtin_interfaces__msg__Time__init(&_time_stamp),
                "Init Time Stamp");

    /*_time_stamp = builtin_interfaces__msg__Time__create();
    NULLCHECK(_time_stamp, "Create Time Stamp Message");

    _cmd_vel_msg = geometry_msgs__msg__Twist__create();
    NULLCHECK(_cmd_vel_msg, "Create Cmd_vel Message");
                
    _imu_msg = sensor_msgs__msg__Imu__create();
    NULLCHECK(_imu_msg, "Create IMU Message");

    _rwheel_state_msg = sensor_msgs__msg__JointState__create();
    NULLCHECK(_rwheel_state_msg, "Create Right Wheel  State Message");

    _lwheel_state_msg = sensor_msgs__msg__JointState__create();
    NULLCHECK(_lwheel_state_msg, "Create Left Wheel  State Message");*/

    RCSOFTCHECK(!sensor_msgs__msg__Imu__init(&_imu_msg),
                "Init Imu Messaage");

    /*RCSOFTCHECK(!(geometry_msgs__msg__Vector3__init(&_imu_msg.angular_velocity)),
                "Init Angular Vel Messaage");
    RCSOFTCHECK(!geometry_msgs__msg__Vector3__init(&_imu_msg.linear_acceleration),
                "Init Acceleration Messaage");*/

    RCSOFTCHECK(!geometry_msgs__msg__Twist__init(&_cmd_vel_msg), 
                "Init CMD Vel");

    _imu_msg.header.frame_id = micro_ros_string_utilities_set(_imu_msg.header.frame_id , "imu_link");
    
    //data for psd, rms are from data sheet
    float accel_psd = 400e-6;
    float accel_bw = 100;
    float accel_var = (std::pow(accel_psd, 2) * accel_bw) * std::pow(9.81, 2);
    float gyro_rms = 0.05 * (M_PI / 180.0);
    float gyro_var = std::pow(gyro_rms, 2);

    for (size_t i = 0; i < 9; ++i) 
    {
        _imu_msg.linear_acceleration_covariance[i] = (i % 4 == 0) ? accel_var : 0.0;
        _imu_msg.angular_velocity_covariance[i] = (i % 4 == 0) ? gyro_var : 0.0;
        _imu_msg.orientation_covariance[i] = -1.0; // -1 indicates no orientation estimate
    }


	const char *lwheel_name = "left_wheel_joint";
	const char *rwheel_name = "right_wheel_joint";

	//Setup the joint state msg
	RCSOFTCHECK(!sensor_msgs__msg__JointState__init(&_lwheel_state_msg),
                "Init Left Wheel State Message");
    RCSOFTCHECK(!sensor_msgs__msg__JointState__init(&_rwheel_state_msg),
                "Init Right Wheel State Message");

	RCSOFTCHECK(!rosidl_runtime_c__double__Sequence__init(&_lwheel_state_msg.position, 1),
                "Init Left Wheel Position Message");
    RCSOFTCHECK(!rosidl_runtime_c__double__Sequence__init(&_rwheel_state_msg.position, 1),
                "Init Right Wheel Position Message");
                
	RCSOFTCHECK(!rosidl_runtime_c__double__Sequence__init(&_lwheel_state_msg.velocity, 1),
                "Init Left Wheel Vel Message");
    RCSOFTCHECK(!rosidl_runtime_c__double__Sequence__init(&_rwheel_state_msg.velocity, 1),
                "Init Right Wheel Vel Message");

	_lwheel_state_msg.position.size = 1;
	_lwheel_state_msg.position.capacity = 1;
    _rwheel_state_msg.position.size = 1;
	_rwheel_state_msg.position.capacity = 1;

    _lwheel_state_msg.velocity.size = 1;
	_lwheel_state_msg.velocity.capacity = 1;
    _rwheel_state_msg.velocity.size = 1;
	_rwheel_state_msg.velocity.capacity = 1;

	//RCSOFTCHECK(!rosidl_runtime_c__String__Sequence__init(&_rwheel_state_msg.name, 1), "Init Left Wheel Name");
	//if (!rosidl_runtime_c__String__assign(&_lwheel_state_msg.name.data[0], lwheel_name)){Serial.println("ERROR: Joined assignment failed\n");}
    _lwheel_state_msg.name.size=1;
    _lwheel_state_msg.name.capacity=1;

    RCSOFTCHECK(rosidl_runtime_c__String__assignn(_lwheel_state_msg.name.data, lwheel_name, sizeof(lwheel_name)), "Assign Left Wheel Name");
    RCSOFTCHECK(rosidl_runtime_c__String__assignn(_rwheel_state_msg.name.data, lwheel_name, sizeof(lwheel_name)), "Assign Right Wheel Name");

    //RCSOFTCHECK(!rosidl_runtime_c__String__Sequence__init(&_rwheel_state_msg.name, 1), "Init Right Wheel Name");
	//if (!rosidl_runtime_c__String__assign(&_rwheel_state_msg.name.data[0], rwheel_name)){Serial.println("ERROR: Joined assignment failed\n");}
    _rwheel_state_msg.name.size=std::string(rwheel_name).length();
    _rwheel_state_msg.name.capacity=sizeof(rwheel_name);
	
}
void ROSComm::PublishCallback(step *lmotor, step *rmotor, Adafruit_MPU6050 *imu)
{

    getData(lmotor, rmotor, imu);

    RCSOFTCHECK(rcl_publish(&_left_wheel_state_pub, &_lwheel_state_msg, NULL), 
                "Publish Left Wheel State");

    RCSOFTCHECK(rcl_publish(&_right_wheel_state_pub, &_rwheel_state_msg, NULL), 
                "Publish Right Wheel State");

    RCSOFTCHECK(rcl_publish(&_imu_pub, &_imu_msg, NULL), 
                "Publish IMU Data");       

}

void ROSComm::getData(step *lmotor, step *rmotor, Adafruit_MPU6050 *imu)
{
    sensors_event_t a, g, temp;
    //imu->getEvent(&a, &g, &temp);

    int64_t now = esp_timer_get_time();
    _time_stamp.sec = now / 1000000;
    _time_stamp.nanosec = (now % 1000000) * 1000;

    RCSOFTCHECK(!builtin_interfaces__msg__Time__copy(&_time_stamp, &_imu_msg.header.stamp),
    "Copy Time TO IMU Message");

    _imu_msg.linear_acceleration.x = 0.02;//a.acceleration.x;
    _imu_msg.linear_acceleration.y = 0.01; //a.acceleration.y;
    _imu_msg.linear_acceleration.z = -9.81; //a.acceleration.z;

    _imu_msg.angular_velocity.x = 0.0; //g.gyro.x;
    _imu_msg.angular_velocity.y = 0.0; //g.gyro.y;
    _imu_msg.angular_velocity.z = 0.5; //g.gyro.z;

    /*_imu_msg->linear_acceleration.x = 0.02;//a.acceleration.x;
    _imu_msg->linear_acceleration.y = 0.01; //a.acceleration.y;
    _imu_msg->linear_acceleration.z = -9.81; //a.acceleration.z;

    _imu_msg->angular_velocity.x = 0.0; //g.gyro.x;
    _imu_msg->angular_velocity.y = 0.0; //g.gyro.y;
    _imu_msg->angular_velocity.z = 0.5; //g.gyro.z;*/


    float lmotor_pos = -1.0; //lmotor->getPositionRad();
    float lmotor_speed = -3.0; //lmotor->getSpeedRad();

    now = esp_timer_get_time();
    _time_stamp.sec = now / 1000000;
    _time_stamp.nanosec = (now % 1000000) * 1000;

    /*now = esp_timer_get_time();
    _time_stamp->sec = now / 1000000;
    _time_stamp->nanosec = (now % 1000000) * 1000;*/

    RCSOFTCHECK(!builtin_interfaces__msg__Time__copy(&_time_stamp, &_lwheel_state_msg.header.stamp),
    "Copy Time TO Left Wheel State Message");

    _lwheel_state_msg.position.data[0] = lmotor_pos;
    _lwheel_state_msg.velocity.data[0] = lmotor_speed;

    /*_lwheel_state_msg->position.data[0] = lmotor_pos;
    _lwheel_state_msg->velocity.data[0] = lmotor_speed;*/


    float rmotor_pos = 1.0; //rmotor->getPositionRad();
    float rmotor_speed = 3.0; //rmotor->getSpeedRad();

    /*now = esp_timer_get_time();
    _time_stamp->sec = now / 1000000;
    _time_stamp->nanosec = (now % 1000000) * 1000;/*/

    now = esp_timer_get_time();
    _time_stamp.sec = now / 1000000;
    _time_stamp.nanosec = (now % 1000000) * 1000;
    
    RCSOFTCHECK(!builtin_interfaces__msg__Time__copy(&_time_stamp, &_rwheel_state_msg.header.stamp),
    "Copy Time TO Right Wheel State Message");

    _rwheel_state_msg.position.data[0] = rmotor_pos;
    _rwheel_state_msg.velocity.data[0] = rmotor_speed;

    /*_rwheel_state_msg->position.data[0] = rmotor_pos;
    _rwheel_state_msg->velocity.data[0] = rmotor_speed;*/
}

rosidl_runtime_c__String ROSComm::getFrameId(const char *input)
{
    rosidl_runtime_c__String ros_string;

    size_t len = std::strlen(input);
    ros_string.size = len;
    ros_string.capacity = len + 1; // Including null byte
    ros_string.data = new char[ros_string.capacity];

    std::strcpy(ros_string.data, input);

    return ros_string;
}