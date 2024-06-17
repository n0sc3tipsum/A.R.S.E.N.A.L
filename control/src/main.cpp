// libraries or packages used
#include <Arduino.h>
#include <TimerInterrupt_Generic.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <step.h>
#include <WiFi.h>
#include <WebServer.h>
#include <net.h>
#include <constant.h>
#include <filter.h>


// The Stepper pins
#define STEPPER1_DIR_PIN 16   //Arduino D9
#define STEPPER1_STEP_PIN 17  //Arduino D8
#define STEPPER2_DIR_PIN 4    //Arduino D11
#define STEPPER2_STEP_PIN 14  //Arduino D10
#define STEPPER_EN 15         //Arduino D12

// Diagnostic pin for oscilloscope
#define TOGGLE_PIN  32        //Arduino A4



//Global objects
ESP32Timer ITimer(3);
Adafruit_MPU6050 mpu;         //Default pins for I2C are SCL: IO22/Arduino D3, SDA: IO21/Arduino D4

step step1(STEPPER_INTERVAL_US,STEPPER1_STEP_PIN,STEPPER1_DIR_PIN );
step step2(STEPPER_INTERVAL_US,STEPPER2_STEP_PIN,STEPPER2_DIR_PIN );

//Interrupt Service Routine for motor update
//Note: ESP32 doesn't support floating point calculations in an ISR
bool TimerHandler(void * timerNo)
{
  static bool toggle = false;

  //Update the stepper motors
  step1.runStepper();
  step2.runStepper();

  //Indicate that the ISR is running
  digitalWrite(TOGGLE_PIN,toggle);  
  toggle = !toggle;
	return true;
}

void setup()
{
  Serial.begin(115200);
  pinMode(TOGGLE_PIN,OUTPUT);
  setupNetwork(ssid, password);

  // Try to initialize Accelerometer/Gyroscope
  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    while (1) {
      delay(10);
    }
  }
  Serial.println("MPU6050 Found!");

  mpu.setAccelerometerRange(MPU6050_RANGE_2_G);
  mpu.setGyroRange(MPU6050_RANGE_250_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_44_HZ);
  
  


  //initialize the network


  //Attach motor update ISR to timer to run every STEPPER_INTERVAL_US μs
  if (!ITimer.attachInterruptInterval(STEPPER_INTERVAL_US, TimerHandler)) {
    Serial.println("Failed to start stepper interrupt");
    while (1) delay(10);
  }
  Serial.println("Initialised Interrupt for Stepper");

  // Set motor acceleration values
  step1.setAccelerationRad(13.0);
  step2.setAccelerationRad(13.0);

  //Enable the stepper motor drivers
  pinMode(STEPPER_EN,OUTPUT);
  digitalWrite(STEPPER_EN, false);
}

void loop()
{
  //Static variables are initialised once and then the value is remembered betweeen subsequent calls to this function
  static unsigned long printTimer = 0;  //time of the next print
  static unsigned long loopTimer = 0;   //time of the next control update
  static unsigned long SpeedTimer = 0;
  // static float tiltx = 0.0;             //current tilt angle
  
  // loop for outer loop

  if (millis() > SpeedTimer){
    SpeedTimer += SPEED_INTERVAL;

    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);

    //get the average linear velocity
    GetSpeed = (step1.getSpeedRad() + step2.getSpeedRad())/2;
    CurrSpeed = GetSpeed;

    //Implementation of PID for the outer loop linear velocity control
    SpeedError = SetSpeed - CurrSpeed;
    SpeedDerivative = (SpeedError - PreviousSpeedError) / dtSpeed;
    SpeedIntegral += SpeedError* dtSpeed;
    // Connecting the velocity control and the inner loop control by having the output as the reference input
    setpoint = -(SpeedError * KpSpeed + SpeedDerivative*KdSpeed + KiSpeed * SpeedIntegral);
    PreviousSpeedError = SpeedError;
    PreviousSpeed = CurrSpeed;
  }


  //loop  for the inner loop
  if (millis() > loopTimer) {
    loopTimer += LOOP_INTERVAL;

    // Fetch data from MPU6050
    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);

    // //Calculate Tilt using accelerometer and sin x = x approximation for a small tilt angle
    accelTilt = a.acceleration.z/9.67 - 0.14;
    gyroRate = g.gyro.y -0.015;
    gyroX = g.gyro.roll + 0.09;

    // Implementation of outerloop yaw velocity control
    RotationError = RotationSetpoint - gyroX;
    RotationIntegral+= RotationError * dt;
    RotationControl = RotateP * RotationError + RotationIntegral*RotateI;

    //implementation of inner loop tilt control
    tilt = CompFilter(accelTilt, gyroRate, alpha, tilt);
    error = setpoint - tilt;
    integral += error *dt;

    //connecting the output of the controller together
    PIDout = error * Kp  - gyroRate*Kd + integral * Ki;
    step1.setAccelerationRad(PIDout+RotationControl);
    step2.setAccelerationRad(PIDout-RotationControl);
    if (PIDout < 0){
      step1.setTargetSpeedRad(-20);
      step2.setTargetSpeedRad(-20);
      

    }
    else{
    step1.setTargetSpeedRad(20);
    step2.setTargetSpeedRad(20);

    }
  }
  
  // Print updates every PRINT_INTERVAL ms
  if (millis() > printTimer) {
    server.handleClient();
    printTimer += PRINT_INTERVAL;
    // Serial.println(RotationSetpoint);
    // Serial.print(printTimer);
    // Serial.print(" ");
    // Serial.println(gyroX);
    // Serial.print(gyroRate);
    // Serial.println(tilt);
    delay(10);
  }
}