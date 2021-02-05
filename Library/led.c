#include "led.h"
#include "gpio.h"

#include <hw/template_appliance.h>

#define LED_ON GPIO_Value_Low
#define LED_OFF GPIO_Value_High

void OpenLeds(leds_t *leds)
{
    #ifdef TEMPLATE_LED_1_RED
    leds->one.red = OpenGPIO(TEMPLATE_LED_1_RED, LED_OFF);
    #endif
    #ifdef TEMPLATE_LED_1_GREEN
    leds->one.green = OpenGPIO(TEMPLATE_LED_1_GREEN, LED_OFF);
    #endif
    #ifdef TEMPLATE_LED_1_BLUE
    leds->one.blue = OpenGPIO(TEMPLATE_LED_1_BLUE, LED_OFF);
    #endif

    #ifdef TEMPLATE_LED_2_RED
    leds->two.red = OpenGPIO(TEMPLATE_LED_2_RED, LED_OFF);
    #endif
    #ifdef TEMPLATE_LED_2_GREEN
    leds->two.green = OpenGPIO(TEMPLATE_LED_2_GREEN, LED_OFF);
    #endif
    #ifdef TEMPLATE_LED_2_BLUE
    leds->two.blue = OpenGPIO(TEMPLATE_LED_2_BLUE, LED_OFF);
    #endif

    #ifdef TEMPLATE_LED_3_RED
    leds->three.red = OpenGPIO(TEMPLATE_LED_3_RED, LED_OFF);
    #endif
    #ifdef TEMPLATE_LED_3_GREEN
    leds->three.green = OpenGPIO(TEMPLATE_LED_3_GREEN, LED_OFF);
    #endif
    #ifdef TEMPLATE_LED_3_BLUE
    leds->three.blue = OpenGPIO(TEMPLATE_LED_3_BLUE, LED_OFF);
    #endif

    #ifdef TEMPLATE_LED_4_RED
    leds->four.red = OpenGPIO(TEMPLATE_LED_4_RED, LED_OFF);
    #endif
    #ifdef TEMPLATE_LED_4_GREEN
    leds->four.green = OpenGPIO(TEMPLATE_LED_4_GREEN, LED_OFF);
    #endif
    #ifdef TEMPLATE_LED_4_BLUE
    leds->four.blue = OpenGPIO(TEMPLATE_LED_4_BLUE, LED_OFF);
    #endif

    #ifdef TEMPLATE_LED_NETWORKING_RED
    leds->wifi.red = OpenGPIO(TEMPLATE_LED_NETWORKING_RED, LED_OFF);
    #endif
    #ifdef TEMPLATE_LED_NETWORKING_GREEN
    leds->wifi.green = OpenGPIO(TEMPLATE_LED_NETWORKING_GREEN, LED_OFF);
    #endif
    #ifdef TEMPLATE_LED_NETWORKING_BLUE
    leds->wifi.blue = OpenGPIO(TEMPLATE_LED_NETWORKING_BLUE, LED_OFF);
    #endif

    #ifdef TEMPLATE_LED_STATUS_RED
    leds->status.red = OpenGPIO(TEMPLATE_LED_STATUS_RED, LED_OFF);
    #endif
    #ifdef TEMPLATE_LED_STATUS_GREEN
    leds->status.green = OpenGPIO(TEMPLATE_LED_STATUS_GREEN, LED_OFF);
    #endif
    #ifdef TEMPLATE_LED_STATUS_BLUE
    leds->status.blue = OpenGPIO(TEMPLATE_LED_STATUS_BLUE, LED_OFF);
    #endif
}

void CloseLeds(leds_t *leds)
{
    #if(defined TEMPLATE_LED_1_RED && defined TEMPLATE_LED_1_GREEN && defined TEMPLATE_LED_1_BLUE)
    SetLedState(leds->one, false, false, false);
    #endif
    #if(defined TEMPLATE_LED_2_RED && defined TEMPLATE_LED_2_GREEN && defined TEMPLATE_LED_2_BLUE)
    SetLedState(leds->two, false, false, false);
    #endif
    #if(defined TEMPLATE_LED_3_RED && defined TEMPLATE_LED_3_GREEN && defined TEMPLATE_LED_3_BLUE)
    SetLedState(leds->three, false, false, false);
    #endif
    #if(defined TEMPLATE_LED_4_RED && defined TEMPLATE_LED_4_GREEN && defined TEMPLATE_LED_4_BLUE)
    SetLedState(leds->four, false, false, false);
    #endif
    #if(defined TEMPLATE_LED_NETWORKING_RED && defined TEMPLATE_LED_NETWORKING_GREEN && defined TEMPLATE_LED_NETWORKING_BLUE)
    SetLedState(leds->wifi, false, false, false);
    #endif
    #if(defined TEMPLATE_LED_STATUS_RED && defined TEMPLATE_LED_STATUS_GREEN && defined TEMPLATE_LED_STATUS_BLUE)
    SetLedState(leds->status, false, false, false);
    #endif
}

void SetLedState(led_channels_t channel, bool red, bool green, bool blue)
{
    GPIO_SetValue(channel.red, red ? LED_ON : LED_OFF);
    GPIO_SetValue(channel.green, green ? LED_ON : LED_OFF);
    GPIO_SetValue(channel.blue, blue ? LED_ON : LED_OFF);
}

void SetLedChannelFdState(int channelFd, bool state) 
{
    GPIO_SetValue(channelFd, state ? LED_ON : LED_OFF);
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