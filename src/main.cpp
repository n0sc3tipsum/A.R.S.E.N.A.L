#include <Arduino.h>
#include <WiFi.h>
#include <IPAddress.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include <constant.h>
#include <filter.h>
#include <ESP32TimerInterrupt_Generic.h>

#include "Battery.h"
#include "ROSComm.h"
#include "step.h"

float *RotationSetpoint;
float *SetSpeed;
step left_motor  = step(STEPPER_INTERVAL_US, STEPPER1_STEP_PIN, STEPPER1_DIR_PIN);
step right_motor = step(STEPPER_INTERVAL_US, STEPPER2_STEP_PIN, STEPPER2_DIR_PIN);

imu_data_t *imu_data; 
Adafruit_MPU6050 imu;
Battery batt;
ROSComm espRosAgent;

ESP32Timer ITimer(3);

//Interrupt Service Routine for motor update
//Note: ESP32 doesn't support floating point calculations in an ISR
bool TimerHandler(void * timerNo)
{
    static bool toggle = false;

    //Update the stepper motors
    left_motor.runStepper();
    right_motor.runStepper();

    //Indicate that the ISR is running
    digitalWrite(TOGGLE_PIN,toggle);  
    toggle = !toggle;
    return true;
}

void timer_callback(rcl_timer_t * timer, int64_t last_call_time)
{
    espRosAgent.PublishCallback(&left_motor, &right_motor, *imu_data, batt.BatteryLevel, batt.TotalPower);
}

void cmd_vel_sub_callback(const void *msgin)
{
    espRosAgent.CommandCallback(msgin, RotationSetpoint, SetSpeed);
}

void cleanup()
{
    espRosAgent.Cleanup();
}

void setup()
{
    Serial.begin(115200);
/*
    pinMode(TOGGLE_PIN,OUTPUT);

    // Try to initialize Accelerometer/Gyroscope
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


    //Attach motor update ISR to timer to run every STEPPER_INTERVAL_US μs
    if (!ITimer.attachInterruptInterval(STEPPER_INTERVAL_US, TimerHandler)) 
    {
        Serial.println("Failed to start stepper interrupt");
        while (1) delay(10);
    }
    Serial.println("Initialised Interrupt for Stepper");

 
    //Enable the stepper motor drivers
    //pinMode(STEPPER_EN,OUTPUT);
    //digitalWrite(STEPPER_EN, false);
    delay(2000);
*/
    WiFi.begin(espRosAgent._ssid, espRosAgent._pswd);

    while (WiFi.status() != WL_CONNECTED) 
    {
        delay(500);
        Serial.println("Connecting to WiFi..");
    }

    espRosAgent._agent_ip = IPAddress(192,168,191,78);
    espRosAgent._esp_ip = WiFi.localIP();
    Serial.print("Connected to WiFi with local IP : ");
    Serial.println(espRosAgent._esp_ip);

    *RotationSetpoint = 0.0;
    *SetSpeed = 0.0;
    espRosAgent.Init();

    const unsigned int timer_timeout = 500;

    RCSOFTCHECK(rclc_timer_init_default(
                &espRosAgent.timer,
                &espRosAgent.support,
                RCL_MS_TO_NS(timer_timeout),
                timer_callback), 
                "Init Timer");


    RCSOFTCHECK(rclc_executor_add_subscription(
                &espRosAgent.executor, 
                &espRosAgent._cmd_vel_sub, &espRosAgent._cmd_vel_msg, 
                &cmd_vel_sub_callback, ON_NEW_DATA), 
                "Create cmd_vel Subscription");

    RCSOFTCHECK(rclc_executor_add_timer(&espRosAgent.executor, &espRosAgent.timer), 
                "Add Timer To Executor");

}



void loop()
{
    static unsigned long printTimer = 0;  //time of the next print
    static unsigned long loopTimer = 0;   //time of the next control update

    if (Serial.available() > 0) 
    {
        String command = Serial.readStringUntil('\n');  // Read the command until newline
        command.trim();  // Remove any whitespace or newline characters

        if (command == "q") 
        {           
            Serial.println("------ Shutdown command received. Exiting... -------");
            cleanup();  // Call the cleanup function
            while(true);  // Optionally, enter an infinite loop to stop further execution
        }
    }

    /*if (millis() > loopTimer) 
    {
        loopTimer += LOOP_INTERVAL;

        // Fetch data from MPU6050
        imu.getEvent(&imu_data->accel, &imu_data->gyro, &imu_data->temp);

        GetSpeed = (left_motor.getSpeedRad() + left_motor.getSpeedRad())/2;
        CurrSpeed = LPF(GetSpeed, PreviousSpeed, 0.2);
        SpeedError = *SetSpeed - CurrSpeed;


        SpeedDerivative = (SpeedError - PreviousSpeedError) / dt;
        SpeedIntegral += SpeedError* dt;


        setpoint           = -(SpeedError * KpSpeed + SpeedDerivative*KdSpeed + KiSpeed*SpeedIntegral)-0.04;
        PreviousSpeedError = SpeedError;
        PreviousSpeed      = CurrSpeed;
    

        //Calculate Tilt using accelerometer and sin x = x approximation for a small tilt angle
        accelTilt = imu_data->accel.acceleration.z/9.67;
        gyroRate  = imu_data->gyro.gyro.y;


        tilt = CompFilter(accelTilt, gyroRate, alpha, tilt);
        error = setpoint - tilt;

        integral += error *dt;

        PIDout = error * Kp  - gyroRate*Kd + integral * Ki;
        left_motor.setAccelerationRad(PIDout);
        right_motor.setAccelerationRad(PIDout);

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

        PreviousError = error;

        batt.getBatteryState();
    }*/

    rclc_executor_spin_some(&espRosAgent.executor, RCL_MS_TO_NS(100));
    delay(50);
}