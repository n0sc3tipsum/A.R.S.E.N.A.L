#ifndef BATTERY_H
#define BATTERY_H

#include <Arduino.h>

#define VALUE_TO_VOLTAGE    8.06e-4
#define VOLTAGE_TO_LEVEL    3.63e-3
#define VOLTAGE_TO_CURRENT  2.325

#define MOTOR_PIN           35     //A5
#define LOGIC_PIN           32      //A4
#define BATTERY_PIN         33      //A3


class Battery
{
public:

    Battery();

    void getBatteryState();
    int BatteryLevel;
    int TotalPower;

};

#endif
