#include <stdbool.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <sched.h>

#include <applibs/log.h>
#include <applibs/gpio.h>
#include <applibs/i2c.h>

#include "../../MT3620_Grove_Shield_Library/Grove.h"
#include "../../MT3620_Grove_Shield_Library/Sensors/GroveOledDisplay96x96.h"
#include "../../MT3620_Grove_Shield_Library/Sensors/GroveTempHumiSHT31.h"
#include "../../MT3620_Grove_Shield_Library/Sensors/Grove4DigitDisplay.h"
#include "gpio.h"
#include "led.h"
#include "vibrationMotor.h"
#include "exit.h"
#include "logo.h"
#include "sleep.h"

#include <hw/template_appliance.h>

#define INITIAL_COUNTDOWN 10

static int i2cFd;
static void *sht31;
static leds_t leds;

static void *digitsDisplay;
static int countdown = INITIAL_COUNTDOWN;
static int rotation = 0;

static int vibrationMotor;
static bool vibrationEnabled = false;
static int vibrations = 0;

pthread_mutex_t mutex;

static volatile sig_atomic_t terminationRequested = false;

static void TerminationHandler(int signalNumber)
{
    terminationRequested = true;
}

static void CountdownElapsed(void)
{
    vibrationEnabled = true;
}

static void MaxVibrationsReached(void)
{
    countdown = INITIAL_COUNTDOWN;
    rotation = 0;
}

static void ShowSplashScreen(void)
{
    clearDisplay();
    drawBitmap(logo, sizeof(logo));
}

static void *Blink(void *data)
{
    int colorState = 0;
    unsigned int wait = 100;

    while (!terminationRequested)
    {
        // states:
        // 0 red
        // 1 red + green = yellow
        // 2 green
        // 3 green + blue = light blue
        // 4 blue
        // 5 blue + red = purple
        bool red = colorState == 0 || colorState == 1 || colorState == 5;
        bool green = colorState == 1 || colorState == 2 || colorState == 3;
        bool blue = colorState == 3 || colorState == 4 || colorState == 5;

        SetLedState(leds.one, red, green, blue);
        Sleep(wait);
        SetLedState(leds.one, false, false, false);

        SetLedState(leds.two, red, green, blue);
        Sleep(wait);
        SetLedState(leds.two, false, false, false);

        SetLedState(leds.three, red, green, blue);
        Sleep(wait);
        SetLedState(leds.three, false, false, false);

        SetLedState(leds.four, red, green, blue);
        Sleep(wait);
        SetLedState(leds.four, false, false, false);

        SetLedState(leds.three, red, green, blue);
        Sleep(wait);
        SetLedState(leds.three, false, false, false);

        SetLedState(leds.two, red, green, blue);
        Sleep(wait);
        SetLedState(leds.two, false, false, false);

        colorState = (colorState + 1) % 6;
    }

    SetLedState(leds.one, false, false, false);
    SetLedState(leds.two, false, false, false);
    SetLedState(leds.three, false, false, false);
    SetLedState(leds.four, false, false, false);

    return NULL;
}

static void *TempHumi(void *data)
{
    pthread_mutex_lock(&mutex);
    clearDisplay();
    setNormalDisplay();
    setVerticalMode();
    pthread_mutex_unlock(&mutex);

    while (!terminationRequested)
    {
        pthread_mutex_lock(&mutex);
        GroveTempHumiSHT31_Read(sht31);
        float temp = GroveTempHumiSHT31_GetTemperature(sht31);
        float humi = GroveTempHumiSHT31_GetHumidity(sht31);

        setTextXY(1, 0);
        putString("Temperature:");

        setTextXY(2, 0);
        char text1[16];
        sprintf(text1, "%.1f C", temp);
        putString(text1);

        setTextXY(4, 0);
        putString("Humidity:");

        setTextXY(5, 0);
        char text2[16];
        sprintf(text2, "%.1f %%", humi);
        putString(text2);
        pthread_mutex_unlock(&mutex);

        Sleep(100);
    }

    clearDisplay();

    return NULL;
}

static void *Vibrate(void *data)
{
    while (!terminationRequested)
    {
        if (vibrationEnabled)
        {
            if (vibrations < 3)
            {
                Pulse(vibrationMotor, 200);
                Sleep(100);
                Pulse(vibrationMotor, 200);
                Sleep(100);
                Pulse(vibrationMotor, 200);
                Sleep(100);

                Sleep(500);

                vibrations++;
            }
            else if (vibrations == 3)
            {
                vibrations = 0;
                vibrationEnabled = false;

                MaxVibrationsReached();
            }
        }
    }

    return NULL;
}

#define SEGMENT_CLOCK_POINT 0b10000000
#define SEGMENT_NONE 0b00000000
#define SEGMENT_MIDDLE 0b01000000
#define SEGMENT_TOP_LEFT 0b00100000
#define SEGMENT_BOTTOM_LEFT 0b00010000
#define SEGMENT_BOTTOM 0b00001000
#define SEGMENT_BOTTOM_RIGHT 0b00000100
#define SEGMENT_TOP_RIGHT 0b00000010
#define SEGMENT_TOP 0b00000001

static void *DisplayDigits(void *data)
{
    int segmentsDigit1[] = {SEGMENT_TOP, SEGMENT_NONE, SEGMENT_NONE, SEGMENT_NONE, SEGMENT_NONE, SEGMENT_NONE, SEGMENT_NONE, SEGMENT_NONE, SEGMENT_NONE, SEGMENT_BOTTOM, SEGMENT_BOTTOM_LEFT, SEGMENT_TOP_LEFT};
    int segmentsDigit2[] = {SEGMENT_NONE, SEGMENT_TOP, SEGMENT_NONE, SEGMENT_NONE, SEGMENT_NONE, SEGMENT_NONE, SEGMENT_NONE, SEGMENT_NONE, SEGMENT_BOTTOM, SEGMENT_NONE, SEGMENT_NONE, SEGMENT_NONE};
    int segmentsDigit3[] = {SEGMENT_NONE, SEGMENT_NONE, SEGMENT_TOP, SEGMENT_NONE, SEGMENT_NONE, SEGMENT_NONE, SEGMENT_NONE, SEGMENT_BOTTOM, SEGMENT_NONE, SEGMENT_NONE, SEGMENT_NONE, SEGMENT_NONE};
    int segmentsDigit4[] = {SEGMENT_NONE, SEGMENT_NONE, SEGMENT_NONE, SEGMENT_TOP, SEGMENT_TOP_RIGHT, SEGMENT_BOTTOM_RIGHT, SEGMENT_BOTTOM, SEGMENT_NONE, SEGMENT_NONE, SEGMENT_NONE, SEGMENT_NONE, SEGMENT_NONE};

    while (!terminationRequested)
    {
        if (countdown >= 0)
        {
            Grove4DigitDisplay_DisplayClockPoint(true);

            int seconds = countdown % 60;
            int minutes = countdown / 60;
            Grove4DigitDisplay_DisplayOneSegment(digitsDisplay, 0, (minutes / 10) % 10);
            Grove4DigitDisplay_DisplayOneSegment(digitsDisplay, 1, minutes % 10);
            Grove4DigitDisplay_DisplayOneSegment(digitsDisplay, 2, (seconds / 10) % 10);
            Grove4DigitDisplay_DisplayOneSegment(digitsDisplay, 3, seconds % 10);

            Sleep(1000);

            countdown--;
        }

        if (countdown == -1)
        {
            Grove4DigitDisplay_DisplayClockPoint(false);

            for (; rotation < 3; rotation++)
            {
                for (int step = 0; step < 12; step++)
                {
                    Grove4DigitDisplay_DisplayOneSegmentRaw(digitsDisplay, 0, segmentsDigit1[step]);
                    Grove4DigitDisplay_DisplayOneSegmentRaw(digitsDisplay, 1, segmentsDigit2[step]);
                    Grove4DigitDisplay_DisplayOneSegmentRaw(digitsDisplay, 2, segmentsDigit3[step]);
                    Grove4DigitDisplay_DisplayOneSegmentRaw(digitsDisplay, 3, segmentsDigit4[step]);

                    Sleep(100);
                }
            }

            Grove4DigitDisplay_DisplayOneSegment(digitsDisplay, 0, 13 /* d */);
            Grove4DigitDisplay_DisplayOneSegment(digitsDisplay, 1, 14 /* E */);
            Grove4DigitDisplay_DisplayOneSegment(digitsDisplay, 2, 10 /* A */);
            Grove4DigitDisplay_DisplayOneSegment(digitsDisplay, 3, 13 /* d */);

            CountdownElapsed();
        }
    }

    return NULL;
}

static void InitPeripheralsAndHandlers(void)
{
    struct sigaction action;
    memset(&action, 0, sizeof(struct sigaction));
    action.sa_handler = TerminationHandler;
    sigaction(SIGTERM, &action, NULL);

    GroveShield_Initialize(&i2cFd, 230400);
    GroveOledDisplay_Init(i2cFd, SH1107G);
    sht31 = GroveTempHumiSHT31_Open(i2cFd);

    vibrationMotor = OpenVibrationMotor(TEMPLATE_VIBRATION_MOTOR);

    digitsDisplay = Grove4DigitDisplay_Open(TEMPLATE_4_DIGIT_DISPLAY_CLK, TEMPLATE_4_DIGIT_DISPLAY_DIO);

    OpenLeds(&leds);
}

int main(void)
{
    InitPeripheralsAndHandlers();

    if (pthread_mutex_init(&mutex, NULL) != 0)
    {
        return 1;
    }

    ShowSplashScreen();

    pthread_t blinkThread;
    pthread_create(&blinkThread, NULL, Blink, (void *)NULL);

    pthread_t tempHumiThread;
    pthread_create(&tempHumiThread, NULL, TempHumi, (void *)NULL);

    pthread_t contrastThread;
    pthread_create(&contrastThread, NULL, Vibrate, (void *)NULL);

    pthread_t digitDisplayThread;
    pthread_create(&digitDisplayThread, NULL, DisplayDigits, (void *)NULL);

    pthread_join(blinkThread, NULL);
    pthread_join(tempHumiThread, NULL);
    pthread_join(contrastThread, NULL);
    pthread_join(digitDisplayThread, NULL);

    pthread_mutex_destroy(&mutex);

    return 0;
}