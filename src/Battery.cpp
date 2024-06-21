#include "Battery.h"



Battery::Battery()
{
    initializeData();
    count = 0;
}

void Battery::updatePowerStatus()
{
    int motor_value = analogRead(MOTOR_PIN);
    int logic_value = analogRead(LOGIC_PIN);
    int battery_value = analogRead(BATTERY_PIN);
    /*Serial.println(motor_value);
    Serial.println(logic_value);
    Serial.println(battery_value);*/
  // Calculate motor current
    float motor_voltage = (motor_value / (float)ADC_RESOLUTION) * ADC_MAX_VOLTAGE;
    float motor_diff_volt = motor_voltage / MOTOR_GAIN;
    motor_current += motor_diff_volt / RESISTOR;

  // Calculate logic current
    float logic_voltage = (logic_value / (float)ADC_RESOLUTION) * ADC_MAX_VOLTAGE;
    float logic_diff_volt = logic_voltage / LOGIC_GAIN;
    logic_current += logic_diff_volt / RESISTOR;

  // Calculate battery voltage
    battery_voltage += (battery_value/ (float)ADC_RESOLUTION) * ADC_MAX_VOLTAGE*  BATTERY_GAIN;

  // Store values in arrays
     count++;

  // Check if NUM_SAMPLES (50) iterations are complete
  if (count >= NUM_SAMPLES) 
  {
    // Calculate averages
    motor_average = motor_current/NUM_SAMPLES;
    logic_average = logic_current/NUM_SAMPLES;
    battery_average = battery_voltage/NUM_SAMPLES;
    logic_current = 0;
    battery_voltage = 0;
    motor_current = 0;

    // Perform calculations using averages
    // Example: Calculate power or any other derived value using averages
    float motor_power = motor_average * motor_average * RESISTOR * 1000;
    float logic_power = logic_average * logic_average * RESISTOR * 1000;
    int index = findClosestPercentage(battery_average);
    int correspondingPercentage = percentages[index];
    
    // Reset counters and arrays for next set of readings
    count = 0;

    totalPower = (int)(motor_power + logic_power) ;
    batteryVoltage = (int)battery_average;
    batteryLevel = (int)correspondingPercentage;
  }

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



void Battery::initializeData() {
   for (int i = 0; i < 100; ++i) {
        percentages[i] = 100.0 - i; // 100% to 1%
        voltages[i] = 6.5+ (16.8 - 6.5) * std::sqrt(percentages[i] / 100.0);
    }
    percentages[ 00] = 0.0;
    voltages[100] = 6.5;
}


int Battery::findClosestPercentage(double voltage) {
    int closestIndex = 0;
    double minDifference = std::abs(voltages[0] - voltage);

    for (int i = 1; i < 101; ++i) {
        double difference = std::abs(voltages[i] - voltage);
        if (difference < minDifference) {
            minDifference = difference;
            closestIndex = i;
        }
    }
    return closestIndex;
}
