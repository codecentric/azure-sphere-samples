#include "vibrationMotor.h"
#include "gpio.h"
#include "sleep.h"

#define VIBRATION_MOTOR_ON GPIO_Value_High
#define VIBRATION_MOTOR_OFF GPIO_Value_Low

int OpenVibrationMotor(int vibrationMotorGpioId)
{
    return OpenGPIO(vibrationMotorGpioId, VIBRATION_MOTOR_OFF);
}

void SetVibration(int vibrationMotor, bool on)
{
    GPIO_SetValue(vibrationMotor, on ? VIBRATION_MOTOR_ON : VIBRATION_MOTOR_OFF);
}

void Pulse(int vibrationMotor, unsigned int msec) {
    SetVibration(vibrationMotor, true);
    Sleep(msec);
    SetVibration(vibrationMotor, false);
}