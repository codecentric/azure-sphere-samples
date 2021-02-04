#pragma once

#include <applibs/gpio.h>

int OpenGPIO(int ledGpioLedId, GPIO_Value_Type initialValue);

GPIO_Value_Type ReadGPIO(int gpioId);