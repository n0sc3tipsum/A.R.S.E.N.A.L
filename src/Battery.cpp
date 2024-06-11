#include "Battery.h"

Battery::Battery()
{}

void Battery::getBatteryState()
{
    float pre_battery_votlage = analogRead(BATTERY_PIN) * VALUE_TO_VOLTAGE;
    BatteryLevel = (int)(pre_battery_votlage * VOLTAGE_TO_LEVEL);

    float pre_motor_voltage = analogRead(MOTOR_PIN) * VALUE_TO_VOLTAGE;
    float motor_current     = pre_motor_voltage * VOLTAGE_TO_CURRENT;
    float motor_power       = sq(motor_current) * 0.01;

    // Logic currenr calculation
    float pre_logic_voltage = analogRead(LOGIC_PIN);
    float logic_current     = pre_logic_voltage * VOLTAGE_TO_CURRENT;
    float logic_power       = sq(logic_current) * 0.01;

    TotalPower = (int)(logic_power + motor_power);
}