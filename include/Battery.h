#ifndef BATTERY_H
#define BATTERY_H

#include <Arduino.h>

#define VALUE_TO_VOLTAGE    8.06e-4
#define VOLTAGE_TO_LEVEL    3.63e-3
#define VOLTAGE_TO_CURRENT  2.325

#define MOTOR_PIN           25      //A2
#define LOGIC_PIN           26      //A1
#define BATTERY_PIN         27      //A0


class Battery
{
public:

    Battery();

    void getBatteryState();
    int BatteryLevel;
    int TotalPower;

};

#endif