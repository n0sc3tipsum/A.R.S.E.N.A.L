#ifndef BATTERY_H
#define BATTERY_H

#include <Arduino.h>

#define VALUE_TO_VOLTAGE    8.06e-4
#define VOLTAGE_TO_LEVEL    3.63e-3
#define VOLTAGE_TO_CURRENT  2.325

#define MOTOR_PIN           35      //A2
#define LOGIC_PIN           32      //A1
#define BATTERY_PIN         33      //A0
#define MOTOR_GAIN           150      //A2
#define LOGIC_GAIN           150     //A1
#define BATTERY_GAIN         6.1      //A0
#define ADC_MAX_VOLTAGE           3.3      //A2
#define ADC_RESOLUTION           4095      //A1
#define RESISTOR         0.01      //A0
#define NUM_SAMPLES     100
class Battery
{
public:

    Battery();

    void initializeData();
    int findClosestPercentage(double voltage);
    void updatePowerStatus();
    void printData(float motor_voltage, float logic_voltage, float motor_power, float logic_power, float battery_average, float correspondingPercentage);
    int batteryLevel;
    int batteryVoltage;
    int totalPower;
    int motor_current;
    int logic_current;
    int battery_voltage;
    float motor_average;
    float logic_average;
    float battery_average;

    float motor_values[NUM_SAMPLES];
    float logic_values[NUM_SAMPLES];
    float battery_values[NUM_SAMPLES];
    int percentages[101];
    int voltages[101]; 
    int count;
};

#endif