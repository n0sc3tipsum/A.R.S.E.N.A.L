#include <Arduino.h>
#include <TimerInterrupt_Generic.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <step.h>

// The Stepper pins
#define STEPPER1_DIR_PIN 16   //Arduino D9
#define STEPPER1_STEP_PIN 17  //Arduino D8
#define STEPPER2_DIR_PIN 4    //Arduino D11
#define STEPPER2_STEP_PIN 14  //Arduino D10
#define STEPPER_EN 15         //Arduino D12

// Diagnostic pin for oscilloscope
#define TOGGLE_PIN  32        //Arduino A4

const int PRINT_INTERVAL = 500;
const int LOOP_INTERVAL = 10;
const int  STEPPER_INTERVAL_US = 20;

// complementary filter
float dt = LOOP_INTERVAL / 1000.0;
float alpha = 0.98; // complementary filter coefficient
float accelTilt = 0.0;
// float integralThreshold = 0.5;

// PID constant
const float Kp = 980;
const float Ki = 5.0;
const float Kd = 20.0;

// PID for balancing
float setpoint = -0.03; // desired tilted angle (upright)
float offset = -0.03;
float tilt = 0.0; // current tilt
float gyroRate = 0.0;
float prevTilt = 0.0; //previous tilt for derivative measurement
float integral = 0.0; // integral term
float derivative = 0.0; // derivative term
float error = 0.0;
float PIDout = 0.0;
float CurrSpeed = 0.0;




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
  // static float tiltx = 0.0;             //current tilt angle
  
  //Run the control loop every LOOP_INTERVAL ms
  if (millis() > loopTimer) {
    loopTimer += LOOP_INTERVAL;

    // Fetch data from MPU6050
    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);

    //Calculate Tilt using accelerometer and sin x = x approximation for a small tilt angle
    // tiltx = a.acceleration.z/9.67; 
    // accelTilt = 90- atan2(a.acceleration.x, a.acceleration.z) * 180 / PI;
    accelTilt = a.acceleration.z/9.67;
    gyroRate = g.gyro.pitch;

    tilt = alpha * (tilt + gyroRate * dt) + (1 - alpha) * accelTilt;
    error = setpoint - tilt;
    if (error == 0){
      integral = 0.0;
    }

    
    integral += error * dt;
    

    derivative = (error - prevTilt)/ dt;



    PIDout = error * Kp + derivative*Kd + integral*Ki;

    //Set target motor speed proportional to tilt angle
    //Note: this is for demonstrating accelerometer and motors - it won't work as a balance controller

    
    step1.setAccelerationRad(PIDout);
    step2.setAccelerationRad(PIDout);

    // IntegratedSpeed += PIDout * dt;

    // if (abs(IntegratedSpeed) > 10 ){
    //   IntegratedSpeed = 0;
    // }
    if (PIDout < 0){
      step1.setTargetSpeedRad(-20);
      step2.setTargetSpeedRad(-20);
      

    }
    else{
    step1.setTargetSpeedRad(20);
    step2.setTargetSpeedRad(20);

    }

    step1.runStepper();
    step2.runStepper();
    // step1.runStepper();
    // step2.runStepper();
    
  
    prevTilt = error;
  }
  
  // //Print updates every PRINT_INTERVAL ms
  if (millis() > printTimer) {
    printTimer += PRINT_INTERVAL;
    Serial.print(printTimer);
    Serial.print(" ");
    Serial.print(tilt);
    Serial.println();
  }
}