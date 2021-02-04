#pragma once

#include <stdbool.h>

void* GroveVibrationMotor_Open(int vibrationMotorGpioId);

void GroveVibrationMotor_SetVibration(void *inst, bool on);

void GroveVibrationMotor_Pulse(void *inst, long msec);