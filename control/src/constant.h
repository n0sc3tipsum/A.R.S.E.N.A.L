#ifndef CONSTANT_H
#define CONSTANT_H


const int PRINT_INTERVAL = 100;
const int LOOP_INTERVAL = 10;
const int SPEED_INTERVAL = 50;
const int ROTATION_INVERVAL = 1;
const int  STEPPER_INTERVAL_US = 20;

// complementary filter
float dt = LOOP_INTERVAL / 1000.0;
float dtSpeed = SPEED_INTERVAL / 1000.0;
float alpha = 0.98; // complementary filter coefficient prev  values = 0.98
float accelTilt = 0.0;
// float integralThreshold = 0.5;

// PID constant for tilt
const float Kp = 1800;  //500 600 700 try: 2000 1500 1800 1700
const float Ki = 0.0;  // 5 25
const float Kd = 20.0; // 15 200 10 30

// PID for tilt angle
float setpoint = 0.0; // desired tilted angle (upright) -0.0375
float tilt = 0.0; // current tilt
float gyroRate = 0.0;
float PreviousError = 0.0; //previous tilt for derivative measurement
float integral = 0.0; // integral term
float derivative = 0.0; // derivative term
float error = 0.0;
float PIDout = 0.0;

// PID constant for speed
const float KpSpeed = 0.002; // try 0.5
const float KdSpeed = 0.0; // try 0.0002
const float KiSpeed = 0.002;

//PID for speed
float SetSpeed = 0.0; // desired speed
float SpeedError = 0.0; // the error on the speed 
float CurrSpeed = 0.0;
float PreviousSpeedError = 0.0;
float PreviousSpeed = 0.0;
float GetSpeed = 0.0;
float SpeedDerivative = 0.0;
float SpeedIntegral = 0.0;

//velocity constnat

float AccelRaw = 0.0;


// netcode holder
const char* ssid = "AndroidAP";
const char* password = "nvff0137";

//rotational control (probably not needed)
 float RotationSetpoint = 0.0;
 float RotationError = 0.0;
 float RotationIntegral = 0.0;
 float gyroX = 0.0;
 const float RotateP = 3;
 const float RotateI = 0.0;

 float RotationControl = 0.0;


#endif