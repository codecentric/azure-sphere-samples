#include "GroveVibrationMotor.h"
#include "../Common/Delay.h"

#include <applibs/gpio.h>
#include <stdlib.h>

#define VIBRATION_MOTOR_ON GPIO_Value_High
#define VIBRATION_MOTOR_OFF GPIO_Value_Low

typedef struct
{
    int vibrationMotorGpioId;
} GroveVibrationMotorInstance;

void* GroveVibrationMotor_Open(int vibrationMotorGpioId)
{
    GroveVibrationMotorInstance *this = (GroveVibrationMotorInstance *)malloc(sizeof(GroveVibrationMotorInstance));

    int fd = GPIO_OpenAsOutput(vibrationMotorGpioId, GPIO_OutputMode_PushPull, VIBRATION_MOTOR_OFF);

    if (fd < 0) {
        return NULL;
    }

    this->vibrationMotorGpioId = fd;

    return this;
}

void GroveVibrationMotor_SetVibration(void *inst, bool on)
{
    GroveVibrationMotorInstance *this = (GroveVibrationMotorInstance *)inst;
    GPIO_SetValue(this->vibrationMotorGpioId, on ? VIBRATION_MOTOR_ON : VIBRATION_MOTOR_OFF);
}

void GroveVibrationMotor_Pulse(void *inst, long msec)
{
    GroveVibrationMotorInstance *this = (GroveVibrationMotorInstance *)inst;
    GroveVibrationMotor_SetVibration(this, true);
    usleep(msec);
    GroveVibrationMotor_SetVibration(this, false);
}