#include <Arduino.h>
#include "ROSComm.h"
#include "step.h"
#include <WiFi.h>
#include <IPAddress.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>

// The Stepper pins
#define STEPPER1_DIR_PIN 16   //Arduino D9
#define STEPPER1_STEP_PIN 17  //Arduino D8
#define STEPPER2_DIR_PIN 4    //Arduino D11
#define STEPPER2_STEP_PIN 14  //Arduino D10
#define STEPPER_EN 15         //Arduino D12

const int PRINT_INTERVAL = 500;
const int LOOP_INTERVAL = 10;
const int  STEPPER_INTERVAL_US = 20;


ROSComm espRosAgent;
step left_motor = step(STEPPER_INTERVAL_US, STEPPER1_STEP_PIN, STEPPER1_DIR_PIN);
step right_motor = step(STEPPER_INTERVAL_US, STEPPER2_STEP_PIN, STEPPER2_DIR_PIN);
Adafruit_MPU6050 imu;

void timer_callback(rcl_timer_t * timer, int64_t last_call_time)
{
    espRosAgent.PublishCallback(&left_motor, &right_motor, &imu);
}

void cmd_vel_sub_callback(const void *msgin)
{
    espRosAgent.CommandCallback(msgin);
}

void setup()
{
    Serial.begin(115200);
    delay(2000);
    WiFi.begin(espRosAgent._ssid, espRosAgent._pswd);

    while (WiFi.status() != WL_CONNECTED) 
    {
        delay(500);
        Serial.println("Connecting to WiFi..");
    }

    espRosAgent._agent_ip = IPAddress(192,168,1,107);
    espRosAgent._esp_ip = WiFi.localIP();
    Serial.print("Connected to WiFi with local IP : ");
    Serial.println(espRosAgent._esp_ip);

    espRosAgent.Init();

    const unsigned int timer_timeout = 1000;

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
    delay(100);
    RCSOFTCHECK(rclc_executor_spin_some(&espRosAgent.executor, RCL_MS_TO_NS(100)), "Execute Spin");
}