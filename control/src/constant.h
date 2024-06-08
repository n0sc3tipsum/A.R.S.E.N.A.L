#ifndef CONSTANT_H
#define CONSTANT_H


const int PRINT_INTERVAL = 500;
const int LOOP_INTERVAL = 10;
const int SPEED_INTERVAL = 30;
const int ROTATION_INVERVAL = 1;
const int  STEPPER_INTERVAL_US = 20;

// complementary filter
float dt = LOOP_INTERVAL / 1000.0;
float alpha = 0.98; // complementary filter coefficient
float accelTilt = 0.0;
// float integralThreshold = 0.5;

// PID constant for tilt
const float Kp = 500;
const float Ki = 5.0;
const float Kd = 150.0;

// PID for tilt angle
float setpoint = -0.0375; // desired tilted angle (upright)
float tilt = 0.0; // current tilt
float gyroRate = 0.0;
float PreviousError = 0.0; //previous tilt for derivative measurement
float integral = 0.0; // integral term
float derivative = 0.0; // derivative term
float error = 0.0;
float PIDout = 0.0;

// PID constant for speed
const float KpSpeed = 0.005;
const float KiSpeed = 0.005;
const float KdSpeed = 0.0006;

//PID for speed
float SetSpeed = 0.0; // desired speed
float SpeedError = 0.0; // the error on the speed 
float CurrSpeed = 0.0;
float PreviousSpeed = 0.0;
float SpeedDerivative = 0.0;
float GetSpeed = 0.0;


// netcode holder
const char* ssid = "AndroidAP";
const char* password = "nvff0137";

//rotational control (probably not needed)
 float gyrox = 0.0;
 float RotateP = 1;
 const float RotationSetpoint = 0.0;
 float RotationControl = 0.0;


#endif