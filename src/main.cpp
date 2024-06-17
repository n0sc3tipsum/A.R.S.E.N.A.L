/* General Libraries*/
#include <Arduino.h>
#include <WiFi.h>
#include <IPAddress.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include <constant.h>
#include <filter.h>

/* Realtime Operating System Libraries */
#include <ESP32TimerInterrupt_Generic.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>

/* Custom Headers */
#include "Battery.h"
#include "ROSComm.h"
#include "step.h"

/* Thread Safety and Tasks */
portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;
TaskHandle_t ROSTask;
TaskHandle_t ControlTask;

/* Shared Data */
volatile double sp__left_motor_speed;
volatile double sp__right_motor_speed;
volatile float sp__roll_rate;
volatile float sp__x_vel;

volatile imu_data_t   imu_data; 
volatile motor_data_t motor_data;


/* Timers */
ESP32Timer ITimer(3);


/* Hardware Objects and Helper Classes */
step left_motor  = step(STEPPER_INTERVAL_US, STEPPER1_STEP_PIN, STEPPER1_DIR_PIN);
step right_motor = step(STEPPER_INTERVAL_US, STEPPER2_STEP_PIN, STEPPER2_DIR_PIN);
Adafruit_MPU6050 imu;
Battery batt;
ROSComm espRosAgent;



/*---------------- Interrupt Service Routines ----------------*/

/*
 * @brief   Timer generated interrupt used to turn the stepper motors
 *         
 * @param timerNo : Handle to the ESP32Timer object used in generating interrupt
 * @return        : bool-> true (always)
 * @note          : Runs with a period of 10ms 
*/

bool timerISR__runMotors(void * timerNo)
{
    static bool toggle = false;
    
    /* Move Motors */
    left_motor.runStepper();
    right_motor.runStepper();

    /* Toggle LED to Indicate Running Status */
    digitalWrite(TOGGLE_PIN, toggle);  
    toggle = !toggle;
    
    return true;
}



/*
 * @brief   Timer generated interrupt used to publish data onto ROS2 network
 *         
 * @param timerNo        : Handle to the rcl timer object used in generating interrupt
 * @param last_call_time : Last time the interrupt was generated -> automatically passed by executor object
 * @return               : void
 * @note                 : Used as a callback function that is attatched to the rcl executor object. Runs every 1 sec
*/

void timerISR__publishData(rcl_timer_t * timer, int64_t last_call_time)
{
    espRosAgent.PublishCallback(&motor_data, &imu_data, batt.BatteryLevel, batt.TotalPower);
}



/*
 * @brief   Timer generated interrupt used to publish data onto ROS2 network
 *         
 * @param timerNo        : Handle to the rcl timer object used in generating interrupt
 * @param last_call_time : Last time the interrupt was generated -> automatically passed by executor object
 * @return               : void
 * @note                 : Used as a callback function that is attatched to the rcl executor object. Runs every 1 sec
*/

void getBodySetpointsISR(const void *msgin)
{
    float local_sp__roll_rate, local_sp__x_vel;
    espRosAgent.CommandCallback(msgin, &local_sp__roll_rate, &local_sp__x_vel);

    portENTER_CRITICAL(&mux);
    sp__roll_rate = local_sp__roll_rate;
    sp__x_vel     = local_sp__x_vel;
    portEXIT_CRITICAL(&mux);

}

void getWheelSetpointsISR(const void *msgin)
{
    double local_sp__left_motor_speed, local_sp__right_motor_speed;
    espRosAgent.KinematicCommandCallback(msgin, &local_sp__left_motor_speed, &local_sp__right_motor_speed);

    portENTER_CRITICAL(&mux);
    sp__left_motor_speed  = local_sp__left_motor_speed;
    sp__right_motor_speed = local_sp__right_motor_speed;
    portEXIT_CRITICAL(&mux);
}

void SpinExecutor(void * param)
{
    for (;;)
    {
        rclc_executor_spin_some(&espRosAgent.executor, RCL_MS_TO_NS(100));
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}



void ControlLoop(void * param)
{
    for (;;)
    {
        static unsigned long angular_loop_period = 0;   
        static unsigned long speed_loop_period   = 0;   

        if (!espRosAgent._kin_en) 
        {  
            sensors_event_t a, g, temp;
            imu.getEvent(&a, &g, &temp);

            /* Sample Data */
            portENTER_CRITICAL(&mux);
            imu_data.accel.acceleration.x = a.acceleration.x;
            imu_data.accel.acceleration.y = a.acceleration.y;
            imu_data.accel.acceleration.z = a.acceleration.z;

            imu_data.gyro.gyro.pitch    = g.gyro.pitch;
            imu_data.gyro.gyro.roll     = g.gyro.roll;
            imu_data.gyro.gyro.heading  = g.gyro.heading;
            imu_data.gyro.gyro.y        = g.gyro.y;

            motor_data.left_pos    = left_motor.getPositionRad();
            motor_data.left_speed  = left_motor.getSpeedRad();
            motor_data.right_pos   = right_motor.getPositionRad();
            motor_data.right_speed = right_motor.getSpeedRad();
            portEXIT_CRITICAL(&mux);


            /* Outer Loop */
            if (millis() > speed_loop_period)
            {
                speed_loop_period += SPEED_INTERVAL;

                body_x_vel = (motor_data.left_speed + motor_data.right_speed)/2;
                error__x_vel = sp__x_vel - body_x_vel;


                SpeedDerivative = (error__x_vel - PreviousSpeedError) / dtSpeed;
                SpeedIntegral += error__x_vel* dtSpeed;

                setpoint = -(error__x_vel * KpSpeed + SpeedDerivative*KdSpeed + KiSpeed * SpeedIntegral);
                PreviousSpeedError = error__x_vel;
            }


            /* Inner Loop */
            if (millis() > angular_loop_period)
            {
                angular_loop_period += LOOP_INTERVAL;

                acceleration_tilt    = imu_data.accel.acceleration.z/9.67 - 0.09;
                measured__roll_rate  = imu_data.gyro.gyro.y;

                error__roll_rate       = sp__roll_rate - imu_data.gyro.gyro.roll + 0.1;
                control_input__angular = RotateP * error__roll_rate;

                tilt      = CompFilter(acceleration_tilt, measured__roll_rate, alpha, tilt);
                error     = setpoint - tilt;
                integral += error *dt;

                PIDout   = error * Kp  - measured__roll_rate*Kd + integral * Ki;


                left_motor.setAccelerationRad(PIDout - control_input__angular);
                right_motor.setAccelerationRad(PIDout + control_input__angular);

                if (PIDout < 0)
                {
                    left_motor.setTargetSpeedRad(-20);
                    right_motor.setTargetSpeedRad(-20);
                }

                else
                {
                    left_motor.setTargetSpeedRad(20);
                    right_motor.setTargetSpeedRad(20);
                }

            }

        }

        else
        {   
            left_motor.setTargetSpeedRad(sp__left_motor_speed);
            right_motor.setTargetSpeedRad(sp__right_motor_speed);
        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void cleanup()
{
    espRosAgent.Cleanup();
    Serial.println("");
    Serial.println("---- Clean Up Complete ----");
}

void setup()
{
    Serial.begin(115200);
    pinMode(TOGGLE_PIN, OUTPUT);

    const unsigned int rcl_timer_period = 1000;
    const bool use_kinematic_control = false;

    sp__left_motor_speed  = 0.0;
    sp__right_motor_speed = 0.0;

    sp__roll_rate = 0.0;
    sp__x_vel     = 0.0;

    /* Configure IMU Sensor */
    if (!imu.begin()) 
    {
        Serial.println("Failed to find MPU6050 chip");
        while (1) 
        {
            delay(10);
        }
    }
    Serial.println("MPU6050 Ready!");

    imu.setAccelerometerRange(MPU6050_RANGE_2_G);
    imu.setGyroRange(MPU6050_RANGE_250_DEG);
    imu.setFilterBandwidth(MPU6050_BAND_44_HZ);


    /* Setup Motors */
    if (!ITimer.attachInterruptInterval(STEPPER_INTERVAL_US, timerISR__runMotors)) 
    {
        Serial.println("Failed to start stepper interrupt");
        while (1) delay(10);
    }
    Serial.println("Initialised Interrupt for Stepper");

    pinMode(STEPPER_EN, OUTPUT);
    digitalWrite(STEPPER_EN, false);


    /* Setup WiFi Connection*/
    WiFi.begin(espRosAgent._ssid, espRosAgent._pswd);

    while (WiFi.status() != WL_CONNECTED) 
    {
        delay(500);
        Serial.println("Connecting to WiFi..");
    }

    espRosAgent._agent_ip = IPAddress(192,168,246,236); //Set this to your desktop IP
    espRosAgent._esp_ip = WiFi.localIP();
    Serial.print("Connected to WiFi with local IP : ");
    Serial.println(espRosAgent._esp_ip);
    espRosAgent.Init(use_kinematic_control);

    if (espRosAgent._kin_en)
    {
        left_motor.setAccelerationRad(30);
        right_motor.setAccelerationRad(30);
        
    }
/*--------------- Setup Executor ---------------*/

    RCSOFTCHECK(rclc_timer_init_default(
            &espRosAgent.timer,
            &espRosAgent.support,
            RCL_MS_TO_NS(rcl_timer_period),
            timerISR__publishData), 
            "Init Timer");


    if (espRosAgent._kin_en)
    {
        RCSOFTCHECK(rclc_executor_add_subscription(
            &espRosAgent.executor, 
            &espRosAgent._kinematic_cmd_vel_sub, &espRosAgent._kinematic_cmd_msg, 
            &getBodySetpointsISR, ON_NEW_DATA), 
            "Create Kinematic Cmd Subscription");
    }

    else
    {
        RCSOFTCHECK(rclc_executor_add_subscription(
            &espRosAgent.executor, 
            &espRosAgent._cmd_vel_sub, &espRosAgent._cmd_vel_msg, 
            &getWheelSetpointsISR, ON_NEW_DATA), 
            "Create cmd_vel Subscription");
    }

    RCSOFTCHECK(rclc_executor_add_timer(&espRosAgent.executor, &espRosAgent.timer), 
                "Add Timer To Executor");


/*--------------- Init Tasks ---------------*/

    /*RCLC Executor Runs on Core 0 
        - Priority = 1 < Control Priority
        - Allocated 5kB of Stack Space*/
    xTaskCreatePinnedToCore(
        SpinExecutor,
        "Publish/Subscribe Task",
        5000,
        NULL,
        1,
        &ROSTask,
        0
    );

    /*Control Loop Runs on Core 1 
        - Priority = 2 > Executor Priority
        - Allocated 5kB of Stack Space*/
    xTaskCreatePinnedToCore(
        ControlLoop,
        "Controller Task",
        5000,
        NULL,
        2,
        &ControlTask,
        1
    );
}



void loop(){}
