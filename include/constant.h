#ifndef CONSTANT_H
#define CONSTANT_H

    #define TOGGLE_PIN  32        //Arduino A4
    // The Stepper pins
    #define STEPPER1_DIR_PIN 16   //Arduino D9
    #define STEPPER1_STEP_PIN 17  //Arduino D8
    #define STEPPER2_DIR_PIN 4    //Arduino D11
    #define STEPPER2_STEP_PIN 14  //Arduino D10
    #define STEPPER_EN 15         //Arduino D12


    const int PRINT_INTERVAL = 100;
    const int LOOP_INTERVAL = 10;
    const int SPEED_INTERVAL = 50;
    const int ROTATION_INVERVAL = 1;
    const int  STEPPER_INTERVAL_US = 20;

    // complementary filter
    float dt = LOOP_INTERVAL / 1000.0;
    float dtSpeed = SPEED_INTERVAL / 1000.0;
    float alpha = 0.98; // complementary filter coefficient prev  values = 0.98
    float acceleration_tilt = 0.0;
    // float integralThreshold = 0.5;

    // PID constant for tilt
    const float Kp = 1800;  //500 600 700 try: 2000 1500 1800 1700
    const float Ki = 0.0;  // 5 25
    const float Kd = 20.0; // 15 200 10 30

    // PID for tilt angle
    float setpoint = 0.0; // desired tilted angle (upright) -0.0375
    float tilt = 0.0; // current tilt
    float measured__roll_rate = 0.0;
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
    //float SetSpeed = 0.0; // desired speed
    float error__x_vel = 0.0; // the error on the speed 
    float CurrSpeed = 0.0;
    float PreviousSpeedError = 0.0;
    float PreviousSpeed = 0.0;
    float body_x_vel = 0.0;
    float SpeedDerivative = 0.0;
    float SpeedIntegral = 0.0;

    //velocity constnat

    float AccelRaw = 0.0;



    //rotational control (probably not needed)
    //float RotationSetpoint = 0.0;
    float error__roll_rate = 0.0;
    float RotationIntegral = 0.0;
    float gyroX = 0.0;
    const float RotateP = 0;
    const float RotateI = 0.0;

    float control_input__angular = 0.0;


#endif