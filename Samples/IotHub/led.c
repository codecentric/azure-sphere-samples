#include "led.h"
#include "gpio.h"

#include <hw/template_appliance.h>

#define LED_ON GPIO_Value_Low
#define LED_OFF GPIO_Value_High

void OpenLeds(leds_t *leds)
{
    leds->one.red = OpenGPIO(TEMPLATE_LED_1_RED, LED_OFF);
    leds->one.green = OpenGPIO(TEMPLATE_LED_1_GREEN, LED_OFF);
    leds->one.blue = OpenGPIO(TEMPLATE_LED_1_BLUE, LED_OFF);

    leds->two.red = OpenGPIO(TEMPLATE_LED_2_RED, LED_OFF);
    leds->two.green = OpenGPIO(TEMPLATE_LED_2_GREEN, LED_OFF);
    leds->two.blue = OpenGPIO(TEMPLATE_LED_2_BLUE, LED_OFF);

    leds->three.red = OpenGPIO(TEMPLATE_LED_3_RED, LED_OFF);
    leds->three.green = OpenGPIO(TEMPLATE_LED_3_GREEN, LED_OFF);
    leds->three.blue = OpenGPIO(TEMPLATE_LED_3_BLUE, LED_OFF);

    leds->four.red = OpenGPIO(TEMPLATE_LED_4_RED, LED_OFF);
    leds->four.green = OpenGPIO(TEMPLATE_LED_4_GREEN, LED_OFF);
    leds->four.blue = OpenGPIO(TEMPLATE_LED_4_BLUE, LED_OFF);

    leds->wifi.red = OpenGPIO(TEMPLATE_LED_NETWORKING_RED, LED_OFF);
    leds->wifi.green = OpenGPIO(TEMPLATE_LED_NETWORKING_GREEN, LED_OFF);
    leds->wifi.blue = OpenGPIO(TEMPLATE_LED_NETWORKING_BLUE, LED_OFF);

    leds->status.red = OpenGPIO(TEMPLATE_LED_STATUS_RED, LED_OFF);
    leds->status.green = OpenGPIO(TEMPLATE_LED_STATUS_GREEN, LED_OFF);
    leds->status.blue = OpenGPIO(TEMPLATE_LED_STATUS_BLUE, LED_OFF);
}

void CloseLeds(leds_t *leds)
{
    SetLedState(leds->one, false, false, false);
    SetLedState(leds->two, false, false, false);
    SetLedState(leds->three, false, false, false);
    SetLedState(leds->four, false, false, false);
    SetLedState(leds->wifi, false, false, false);
    SetLedState(leds->status, false, false, false);
}

void SetLedState(led_channels_t channel, bool red, bool green, bool blue)
{
    GPIO_SetValue(channel.red, red ? LED_ON : LED_OFF);
    GPIO_SetValue(channel.green, green ? LED_ON : LED_OFF);
    GPIO_SetValue(channel.blue, blue ? LED_ON : LED_OFF);
}

bool GetLedRedState(led_channels_t channel)
{
    return ReadGPIO(channel.red) == LED_ON;
}

bool GetLedGreenState(led_channels_t channel)
{
    return ReadGPIO(channel.green) == LED_ON;
}

bool GetLedBlueState(led_channels_t channel)
{
    return ReadGPIO(channel.blue) == LED_ON;
}

bool GetLedFdState(int fd)
{
    return ReadGPIO(fd) == LED_ON;
}