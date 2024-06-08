#include "ROSComm.h"


ROSComm::ROSComm()
{
    _agent_port = 8888;
    _ssid = "BA36 Hyperoptic 1Gbps Broadband";
    _pswd = "pkuusr5x";
 
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
    CreateMessages(_imu_msg, _lwheel_state_msg, _rwheel_state_msg);

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
		"sensor_msgs/BetteryState"),
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
        "Init cmd_vel Subsscriber");
}

void ROSComm::CreateMessages(sensor_msgs__msg__Imu &imu_data, sensor_msgs__msg__JointState &left_wheel_state, sensor_msgs__msg__JointState &right_wheel_state)
{
    Serial.println("Creating Messages");

    RCSOFTCHECK(!builtin_interfaces__msg__Time__init(&_time_stamp),
                "Init Time Stamp");

    RCSOFTCHECK(!geometry_msgs__msg__Twist__init(&_cmd_vel_msg),
                "Init cmd_vel Message");

    RCSOFTCHECK(!sensor_msgs__msg__Imu__init(&imu_data),
                "Init cmd_vel Messaage");
    RCSOFTCHECK(!(geometry_msgs__msg__Vector3__init(&imu_data.angular_velocity)),
                "Init Angular Vel Messaage");
    RCSOFTCHECK(!geometry_msgs__msg__Vector3__init(&imu_data.linear_acceleration),
                "Init Acceleration Messaage");

    //data for psd, rms are from data sheet
    float accel_psd = 400e-6;
    float accel_bw = 100;
    float accel_var = (std::pow(accel_psd, 2) * accel_bw) * std::pow(9.81, 2);
    float gyro_rms = 0.05 * (M_PI / 180.0);
    float gyro_var = std::pow(gyro_rms, 2);

    for (size_t i = 0; i < 9; ++i) 
    {
        imu_data.linear_acceleration_covariance[i] = (i % 4 == 0) ? accel_var : 0.0;
        imu_data.angular_velocity_covariance[i] = (i % 4 == 0) ? gyro_var : 0.0;
        imu_data.orientation_covariance[i] = -1.0; // -1 indicates no orientation estimate
    }



	const char *lwheel_name = "left_wheel_joint";
	const char *rwheel_name = "right_wheel_joint";

	//Setup the joint state msg
	RCSOFTCHECK(!sensor_msgs__msg__JointState__init(&left_wheel_state),
                "Init Left Wheel State Message");
    RCSOFTCHECK(!sensor_msgs__msg__JointState__init(&right_wheel_state),
                "Init Right Wheel State Message");

	RCSOFTCHECK(!rosidl_runtime_c__double__Sequence__init(&left_wheel_state.position, 1),
                "Init Left Wheel Position Message");
    RCSOFTCHECK(!rosidl_runtime_c__double__Sequence__init(&right_wheel_state.position, 1),
                "Init Right Wheel Position Message");
                
	RCSOFTCHECK(!rosidl_runtime_c__double__Sequence__init(&left_wheel_state.velocity, 1),
                "Init Left Wheel Vel Message");
    RCSOFTCHECK(!rosidl_runtime_c__double__Sequence__init(&right_wheel_state.velocity, 1),
                "Init Right Wheel Vel Message");

	left_wheel_state.position.size = 1;
	left_wheel_state.position.capacity = 1;
    right_wheel_state.position.size = 1;
	right_wheel_state.position.capacity = 1;

    left_wheel_state.velocity.size = 1;
	left_wheel_state.velocity.capacity = 1;
    right_wheel_state.velocity.size = 1;
	right_wheel_state.velocity.capacity = 1;

	RCSOFTCHECK(!rosidl_runtime_c__String__Sequence__init(&left_wheel_state.name, 1), "Init Left Wheel Name");
	if (!rosidl_runtime_c__String__assign(&left_wheel_state.name.data[0], lwheel_name)){Serial.println("ERROR: Joined assignment failed\n");}
    left_wheel_state.name.size=1;
    left_wheel_state.name.capacity=1;

    RCSOFTCHECK(!rosidl_runtime_c__String__Sequence__init(&right_wheel_state.name, 1), "Init Right Wheel Name");
	if (!rosidl_runtime_c__String__assign(&right_wheel_state.name.data[0], rwheel_name)){Serial.println("ERROR: Joined assignment failed\n");}
    right_wheel_state.name.size=1;
    right_wheel_state.name.capacity=1;
	
}
void ROSComm::PublishCallback(step *lmotor, step *rmotor, Adafruit_MPU6050 *imu)
{

    getData(_imu_msg, _lwheel_state_msg, _rwheel_state_msg, lmotor, rmotor, imu);

    RCSOFTCHECK(rcl_publish(&_left_wheel_state_pub, &_lwheel_state_msg, NULL), 
                "Publish Left Wheel State");

    RCSOFTCHECK(rcl_publish(&_right_wheel_state_pub, &_rwheel_state_msg, NULL), 
                "Publish Right Wheel State");

    RCSOFTCHECK(rcl_publish(&_imu_pub, &_imu_msg, NULL), 
                "Publish IMU Data");       

}

void ROSComm::getData(sensor_msgs__msg__Imu &imu_data, sensor_msgs__msg__JointState &left_wheel_state, sensor_msgs__msg__JointState &right_wheel_state, 
                      step *lmotor, step *rmotor, Adafruit_MPU6050 *imu)
{
    sensors_event_t a, g, temp;
    //imu->getEvent(&a, &g, &temp);

    int64_t now = esp_timer_get_time();
    _time_stamp.sec = now / 1000000;
    _time_stamp.nanosec = (now % 1000000) * 1000;

    RCSOFTCHECK(!builtin_interfaces__msg__Time__copy(&_time_stamp, &imu_data.header.stamp),
    "Copy Time TO IMU Message");

    imu_data.linear_acceleration.x = 0.02;//a.acceleration.x;
    imu_data.linear_acceleration.y = 0.01; //a.acceleration.y;
    imu_data.linear_acceleration.z = -9.81; //a.acceleration.z;

    imu_data.angular_velocity.x = 0.0; //g.gyro.x;
    imu_data.angular_velocity.y = 0.0; //g.gyro.y;
    imu_data.angular_velocity.z = 0.5; //g.gyro.z;


    float lmotor_pos = -1.0; //lmotor->getPositionRad();
    float lmotor_speed = -3.0; //lmotor->getSpeedRad();

    now = esp_timer_get_time();
    _time_stamp.sec = now / 1000000;
    _time_stamp.nanosec = (now % 1000000) * 1000;

    RCSOFTCHECK(!builtin_interfaces__msg__Time__copy(&_time_stamp, &left_wheel_state.header.stamp),
    "Copy Time TO Left Wheel State Message");

    left_wheel_state.position.data[0] = lmotor_pos;
    left_wheel_state.velocity.data[0] = lmotor_speed;


    float rmotor_pos = 1.0; //rmotor->getPositionRad();
    float rmotor_speed = 3.0; //rmotor->getSpeedRad();

    now = esp_timer_get_time();
    _time_stamp.sec = now / 1000000;
    _time_stamp.nanosec = (now % 1000000) * 1000;

    RCSOFTCHECK(!builtin_interfaces__msg__Time__copy(&_time_stamp, &left_wheel_state.header.stamp),
    "Copy Time TO Left Wheel State Message");

    right_wheel_state.position.data[0] = rmotor_pos;
    right_wheel_state.velocity.data[0] = rmotor_speed;
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