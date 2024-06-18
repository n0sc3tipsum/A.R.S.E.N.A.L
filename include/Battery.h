#ifndef BATTERY_H
#define BATTERY_H

#include <Arduino.h>


#define MOTOR_PIN           35     //A5
#define LOGIC_PIN           32      //A4
#define BATTERY_PIN         33      //A3


class Battery
{
public:

    Battery();

    void getBatteryState();
    int BatteryLevel;
    int getTotalPower;
    int getBatteryVoltage;
    int  getBatteryPercentage;
};

#endif
