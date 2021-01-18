#pragma once

#include <stdbool.h>

int OpenVibrationMotor(int vibrationMotorGpioId);

void SetVibration(int vibrationMotor, bool on);

void Pulse(int vibrationMotor, unsigned int msec);