// Define the pin connected to the analog input
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
#define MOTOR_PIN 25   // A2
#define LOGIC_PIN 26   //A1
#define BATTERY_PIN 27 //A0

const int LOOP_INTERVAL = 50;
const int PRINT_TIMER = 500;

void setup() {
  // Start the serial communication
  Serial.begin(115200);
  
  // Wait for serial port to connect
  while (!Serial) { ; }
  
  // Print a message to indicate that the setup has started
  Serial.println("ESP32 Analog Voltage Reading Started");


}

void loop() {
  // Read the analog value from the specified pin
  static unsigned long looptimer = 0.0;
  static unsigned long print = 0.0;
  int MotorValue = 0.0;
  int LogicValue = 0.0;
  int BatteryValue = 0.0;
  float preBattery_voltage = 0.0;
  float Battery_voltage = 0.0;
  float Battery_level = 0.0;
  float preMotor_voltage = 0.0;
  float Motor_current = 0.0;
  float Motor_power = 0.0;
  float preLogic_voltage = 0.0;
  float Logic_current = 0.0;
  float Logic_power = 0.0;
  float Total_power= 0.0; 
  int prev = 0.0;
  
  if (millis() > looptimer){
  looptimer += LOOP_INTERVAL;
  MotorValue = analogRead(MOTOR_PIN);
  LogicValue = analogRead(LOGIC_PIN);
  BatteryValue = analogRead(BATTERY_PIN);
  
  //Serial.println(analogValue);
  // Convert the analog value to a voltage
  // The default ADC range is 0 to 4095, which corresponds to 0V to 3.3V
  // prev = voltage;

  //Battery Voltage calculation
  preBattery_voltage = BatteryValue * (3.3 / 4095.0);
  Battery_voltage = preBattery_voltage * 6.1;
  Battery_level = (Battery_voltage/ 16.8*100);
  
  //Motor current calculation
  preMotor_voltage = MotorValue * (3.3 / 4095.0);
  Motor_current = preMotor_voltage /(43 * 0.01);
  Motor_power = sq(Motor_current) * 0.01;

  // Logic currenr calculation
  preLogic_voltage = LogicValue * (3.3 / 4095.0);
  Logic_current = preLogic_voltage / (43 * 0.01);
  Logic_power = sq(Logic_current) * 0.01;

  Total_power = Logic_power + Motor_power;
  //Serial.println(voltage);
  // Print the analog value and the corresponding voltage
  
  // Wait for a short period before the next reading
   //delay(1);
  }
  //if ( (millis() > print ) &  (Battery_voltage != 0  )  & ( Motor_current!=0 )   & (Logic_current!=0   )     ){
 
  print += PRINT_TIMER;
  Serial.print("Battery Voltage: ");
  Serial.println(Battery_voltage);

  //Serial.print("Battery Level: ");
  //Serial.println(Battery_level);

  Serial.print("Motor Value: ");
  Serial.println(MotorValue);
  Serial.print("Motor Current: ");
  Serial.println(Motor_current);
  
  //Serial.print("  Voltage: ");
  //Serial.println(voltage);

  Serial.print("Logic Value: ");
  Serial.println(LogicValue);
  Serial.print("Logic Current: ");
  Serial.println(Logic_current);
  // Serial.println("  Voltage: ");
  //Serial.println(voltage);


  //Serial.print("Total_power: ");
  //Serial.println(Total_power);}
//}
}