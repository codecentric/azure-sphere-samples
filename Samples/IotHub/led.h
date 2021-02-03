#pragma once

#include <stdbool.h>

typedef struct led_channels
{
    int red;
    int green;
    int blue;
} led_channels_t;

typedef struct leds
{
    led_channels_t one;
    led_channels_t two;
    led_channels_t three;
    led_channels_t four;
    led_channels_t wifi;
    led_channels_t status;
} leds_t;

void OpenLeds(leds_t *leds);
void CloseLeds(leds_t *leds);

void SetLedState(led_channels_t channel, bool red, bool green, bool blue);

bool GetLedRedState(led_channels_t channel);
bool GetLedGreenState(led_channels_t channel);
bool GetLedBlueState(led_channels_t channel);
bool GetLedFdState(int fd);