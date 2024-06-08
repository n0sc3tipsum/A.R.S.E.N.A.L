// Define the pin connected to the analog input
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_BusIO_Register.h>
#include <Adafruit_Sensor.h>

#define ANALOG_PIN 27

const int LOOP_INTERVAL = 200;
const int PRINT_TIMER = 500;

void setup() {
  // Start the serial communication
  Serial.begin(115200);
  
  // Wait for serial port to connect
  while (!Serial) { ; }
  // Print a message to indicate that the setup has started
  Serial.println("ESP32 Analog Voltage Reading Example");
}

void loop() {
  // Read the analog value from the specified pin
  static unsigned long looptimer = 0.0;
  static unsigned long print = 0.0;
  int analogValue = 0.0;
  float voltage = 0.0;
  int prev = 0.0;
  if (millis() > looptimer){
  looptimer += LOOP_INTERVAL;
  analogValue = analogRead(ANALOG_PIN);
  
  // Convert the analog value to a voltage
  // The default ADC range is 0 to 4095, which corresponds to 0V to 3.3V
  // prev = voltage;
  voltage = analogValue * (3.3 / 4095.0);
  
  // Print the analog value and the corresponding voltage
  
  // Wait for a short period before the next reading
  // delay(1);
  }
  if (millis() > print){
  print += PRINT_TIMER;
  Serial.print("Analog Value: ");
  Serial.print(analogValue);
  Serial.print("  Voltage: ");
  Serial.println(voltage);
  }
}