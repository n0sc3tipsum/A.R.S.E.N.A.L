#include "Battery.h"

Battery::Battery()
{}

#include "Battery.h"



Battery::Battery()
{
    initializeData();
}

void Battery::updatePowerStatus()
{
  int motor_value = analogRead(MOTOR_PIN);
  int logic_value = analogRead(LOGIC_PIN);
  int battery_value = analogRead(BATTERY_PIN);
  
  // Calculate motor current
  float motor_voltage = (motor_value / (float)ADC_RESOLUTION) * ADC_MAX_VOLTAGE;
  float motor_diff_volt = motor_voltage / MOTOR_GAIN;
  float motor_current = motor_diff_volt / RESISTOR;

  // Calculate logic current
  float logic_voltage = (logic_value / (float)ADC_RESOLUTION) * ADC_MAX_VOLTAGE;
  float logic_diff_volt = logic_voltage / LOGIC_GAIN;
  float logic_current = logic_diff_volt / RESISTOR;

  // Calculate battery voltage
  float battery_voltage = (battery_value/ (float)ADC_RESOLUTION) * ADC_MAX_VOLTAGE*  BATTERY_GAIN;

  // Store values in arrays
  motor_values[count] = motor_current;
  logic_values[count] = logic_current;
  battery_values[count] = battery_voltage;
  count++;

  // Check if NUM_SAMPLES (50) iterations are complete
  if (count >= NUM_SAMPLES) {
    // Calculate averages
    float motor_average = calculateAverage(motor_values, NUM_SAMPLES);
    float logic_average = calculateAverage(logic_values, NUM_SAMPLES);
    float battery_average = calculateAverage(battery_values, NUM_SAMPLES);


    // Perform calculations using averages
    // Example: Calculate power or any other derived value using averages
    float motor_power = motor_average * motor_average * RESISTOR* 1000;
    float logic_power = logic_average * logic_average * RESISTOR *1000;
    initializeData();
    int index = findClosestPercentage(battery_average);
    double correspondingPercentage = percentages[index];
    
    // Reset counters and arrays for next set of readings
    count = 0;
    memset(motor_values, 0, sizeof(motor_values));
    memset(logic_values, 0, sizeof(logic_values));
    memset(battery_values, 0, sizeof(battery_values));

    totalPower = (int)(motor_power + logic_power);
    batteryVoltage = (int)battery_average;
    batteryLevel = (int)correspondingPercentage;
  }

  // Delay before next iteration
  //delay(1000);
}


void Battery::printData(float motor_voltage, float logic_voltage, float motor_power, float logic_power, float battery_average, float correspondingPercentage){
    Serial.print("Motor Voltage (using average): ");
    Serial.print(motor_voltage,3);
    Serial.println("V");
    Serial.print("Logic Voltage (using average): ");
    Serial.print(logic_voltage,3);
    Serial.println("V");
    Serial.print("Motor Power (using average): ");
    Serial.print(motor_power,3);
    Serial.println("W");
    Serial.print("Logic Power (using average): ");
    Serial.print(logic_power,3);
    Serial.println("W");
    Serial.print("Battery average): ");
    Serial.print(battery_average,3);
    Serial.println("V");
    Serial.print("The corresponding charge percentage is: " );
    Serial.println(correspondingPercentage,3);
}

int Battery::getBatteryPercentage() {return batteryLevel;}
int Battery::getTotalPower()        {return totalPower;}
int Battery::getBatteryVoltage()    {return batteryVoltage;}


void Battery::initializeData() {
   for (int i = 0; i < SIZE - 1; ++i) {
        percentages[i] = 100.0 - i; // 100% to 1%
        voltages[i] = 6.5+ (16.8 - 6.5) * std::sqrt(percentages[i] / 100.0);
    }
    percentages[SIZE - 1] = 0.0;
    voltages[SIZE - 1] = 6.5;
}

// Function definition to calculate average of an array
float Battery::calculateAverage(float values[], int length) {
  float sum = 0;
  for (int i = 0; i < length; i++) {
    sum += values[i];
  }

  return sum/length;
}

int Battery::findClosestPercentage(double voltage) {
    int closestIndex = 0;
    double minDifference = std::abs(voltages[0] - voltage);

    for (int i = 1; i < SIZE; ++i) {
        double difference = std::abs(voltages[i] - voltage);
        if (difference < minDifference) {
            minDifference = difference;
            closestIndex = i;
        }
    }
    return closestIndex;
}

