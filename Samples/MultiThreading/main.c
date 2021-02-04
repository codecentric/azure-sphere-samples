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

#include <hw/template_appliance.h>

#include "../../MT3620_Grove_Shield_Library/Grove.h"
#include "../../MT3620_Grove_Shield_Library/Sensors/GroveOledDisplay96x96.h"
#include "../../MT3620_Grove_Shield_Library/Sensors/GroveTempHumiSHT31.h"
#include "../../MT3620_Grove_Shield_Library/Sensors/Grove4DigitDisplay.h"
#include "../../MT3620_Grove_Shield_Library/Sensors/GroveI2CMotorDriver.h"
#include "../../MT3620_Grove_Shield_Library/Sensors/GroveVibrationMotor.h"
#include "../../Library/exit.h"
#include "../../Library/gpio.h"
#include "../../Library/led.h"
#include "i2cScanner.h"
#include "logo.h"
#include "state.h"

#define delay(ms) usleep(ms * 1000)

static int i2cFd;
static void *sht31;
static void *digitsDisplay;
static void *vibrationMotor;
static void *motorDriver;

static leds_t leds;

pthread_mutex_t mutex;

store_t *store;

#define lock() pthread_mutex_lock(&mutex)
#define unlock() pthread_mutex_unlock(&mutex)

static volatile sig_atomic_t terminationRequested = false;

static void TerminationHandler(int signalNumber)
{
    terminationRequested = true;
}

static void ShowSplashScreen(void)
{
    clearDisplay();
    drawBitmap(logo, sizeof(logo));
}

static void *Blink(void *data)
{
    int colorState = 0;

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

        lock();
        SetLedState(leds.one, red, green, blue);
        unlock();

        delay(100);

        lock();
        SetLedState(leds.one, false, false, false);
        SetLedState(leds.two, red, green, blue);
        unlock();

        delay(100);

        lock();
        SetLedState(leds.two, false, false, false);
        SetLedState(leds.three, red, green, blue);
        unlock();

        delay(100);

        lock();
        SetLedState(leds.three, false, false, false);
        SetLedState(leds.four, red, green, blue);
        unlock();

        delay(100);

        lock();
        SetLedState(leds.four, false, false, false);
        SetLedState(leds.three, red, green, blue);
        unlock();

        delay(100);

        lock();
        SetLedState(leds.three, false, false, false);
        SetLedState(leds.two, red, green, blue);
        unlock();

        delay(100);

        lock();
        SetLedState(leds.two, false, false, false);
        unlock();

        colorState = (colorState + 1) % 6;

        Log_Debug("Countdown = %d Coffee = %d Loading = %d\r\n", store->state->countdownEnabled, store->state->coffeeRequestEnabled, store->state->loadingIndicatorEnabled);
    }

    SetLedState(leds.one, false, false, false);
    SetLedState(leds.two, false, false, false);
    SetLedState(leds.three, false, false, false);
    SetLedState(leds.four, false, false, false);

    return NULL;
}

static void *ReadTempHumi(void *data)
{
    float temperature = -1;
    float humidity = -1;

    while (!terminationRequested)
    {
        lock();
        GroveTempHumiSHT31_Read(sht31);
        unlock();

        float temp = GroveTempHumiSHT31_GetTemperature(sht31);
        float humi = GroveTempHumiSHT31_GetHumidity(sht31);

        if ((int)(temp * 10) != (int)(temperature * 10))
        {
            Dispatch(store, TEMPERATURE_CHANGED, &temp);
        }

        if ((int)(humi * 10) != (int)(humidity * 10))
        {
            Dispatch(store, HUMIDITY_CHANGED, &humi);
        }

        delay(100);
    }

    return NULL;
}

static void *ShowTempHumi(void *data)
{
    bool alreadyCleared = false;

    while (!terminationRequested)
    {
        float temperature = Temperature(store);
        float humidity = Humidity(store);

        if (temperature != UNKNOWN_VALUE && humidity != UNKNOWN_VALUE)
        {
            lock();

            if (!alreadyCleared)
            {
                alreadyCleared = true;
                clearDisplay();
                setNormalDisplay();
                setVerticalMode();
            }

            setTextXY(1, 0);
            putString("Temperature:");

            setTextXY(2, 0);
            char text1[16];
            sprintf(text1, "%.1f C", temperature);
            putString(text1);

            setTextXY(4, 0);
            putString("Humidity:");

            setTextXY(5, 0);
            char text2[16];
            sprintf(text2, "%.1f %%", temperature);
            putString(text2);
            unlock();
        }

        delay(100);
    }

    clearDisplay();

    return NULL;
}

static void *Vibrate(void *data)
{
    while (!terminationRequested)
    {
        if (VibrationEnabled(store))
        {
            for (int vibrations = 0; vibrations < 3; vibrations++)
            {
                lock();
                GroveVibrationMotor_Pulse(vibrationMotor, 200);
                delay(100);
                GroveVibrationMotor_Pulse(vibrationMotor, 200);
                delay(100);
                GroveVibrationMotor_Pulse(vibrationMotor, 200);
                unlock();

                delay(500);
            }

            Dispatch(store, VIBRATED, (void *)NULL);
        }
    }

    return NULL;
}

static void *ShowCountdown(void *data)
{
    while (!terminationRequested)
    {
        if (CountdownEnabled(store))
        {
            Grove4DigitDisplay_DisplayClockPoint(true);

            for (int8_t countdown = INITIAL_COUNTDOWN; countdown >= 0; countdown--)
            {
                uint8_t seconds = (uint8_t)countdown % 60;
                uint8_t minutes = (uint8_t)countdown / 60;
                uint8_t Mm = (minutes / 10) % 10;
                uint8_t mM = minutes % 10;
                uint8_t Ss = (seconds / 10) % 10;
                uint8_t sS = seconds % 10;

                uint8_t displayData[] = {Mm, mM, Ss, sS};

                lock();
                Grove4DigitDisplay_DisplaySegments(digitsDisplay, displayData);
                unlock();

                delay(1000);
            }

            Dispatch(store, COUNTDOWN_ELAPSED, (void *)NULL);
        }
    }

    return NULL;
}

#define STEPS 12
#define DIGITS 4

int segmentsDigits[STEPS][DIGITS] = {
    {SEGMENT_TOP,
     SEGMENT_NONE,
     SEGMENT_NONE,
     SEGMENT_NONE},
    {SEGMENT_NONE,
     SEGMENT_TOP,
     SEGMENT_NONE,
     SEGMENT_NONE},
    {SEGMENT_NONE,
     SEGMENT_NONE,
     SEGMENT_TOP,
     SEGMENT_NONE},
    {SEGMENT_NONE,
     SEGMENT_NONE,
     SEGMENT_NONE,
     SEGMENT_TOP},
    {SEGMENT_NONE,
     SEGMENT_NONE,
     SEGMENT_NONE,
     SEGMENT_TOP_RIGHT},
    {SEGMENT_NONE,
     SEGMENT_NONE,
     SEGMENT_NONE,
     SEGMENT_BOTTOM_RIGHT},
    {SEGMENT_NONE,
     SEGMENT_NONE,
     SEGMENT_NONE,
     SEGMENT_BOTTOM},
    {SEGMENT_NONE,
     SEGMENT_NONE,
     SEGMENT_BOTTOM,
     SEGMENT_NONE},
    {SEGMENT_NONE,
     SEGMENT_BOTTOM,
     SEGMENT_NONE,
     SEGMENT_NONE},
    {SEGMENT_BOTTOM,
     SEGMENT_NONE,
     SEGMENT_NONE,
     SEGMENT_NONE},
    {SEGMENT_BOTTOM_LEFT,
     SEGMENT_NONE,
     SEGMENT_NONE,
     SEGMENT_NONE},
    {SEGMENT_TOP_LEFT,
     SEGMENT_NONE,
     SEGMENT_NONE,
     SEGMENT_NONE}};

static void *ShowLoadingIndicator(void *data)
{
    while (!terminationRequested)
    {
        if (LoadingIndicatorEnabled(store))
        {
            Grove4DigitDisplay_DisplayClockPoint(false);

            for (int rotation = 0; rotation < 3; rotation++)
            {
                for (int step = 0; step < STEPS; step++)
                {
                    uint8_t displayData[] = {
                        (uint8_t)segmentsDigits[step][0],
                        (uint8_t)segmentsDigits[step][1],
                        (uint8_t)segmentsDigits[step][2],
                        (uint8_t)segmentsDigits[step][3]};
                    lock();
                    Grove4DigitDisplay_DisplaySegmentsRaw(digitsDisplay, displayData);
                    unlock();

                    delay(15);
                }
            }

            Dispatch(store, LOADING_INDICATOR_SHOWN, (void *)NULL);
        }
    }

    return NULL;
}

static void *ShowCoffeeRequest(void *data)
{
    while (!terminationRequested)
    {
        if (CoffeeRequestEnabled(store))
        {
            Grove4DigitDisplay_DisplayClockPoint(false);

            for (int rotation = 0; rotation < 3; rotation++)
            {
                lock();
                Grove4DigitDisplay_DisplayOneSegment(digitsDisplay, 0, 15); // F
                Grove4DigitDisplay_DisplayOneSegment(digitsDisplay, 1, 14); // E
                Grove4DigitDisplay_DisplayOneSegment(digitsDisplay, 2, 14); // E
                Grove4DigitDisplay_DisplayOneSegment(digitsDisplay, 3, 13); // D
                unlock();

                delay(1500);

                lock();
                Grove4DigitDisplay_DisplayOneSegment(digitsDisplay, 0, 12); // C
                Grove4DigitDisplay_DisplayOneSegment(digitsDisplay, 1, 10); // A
                Grove4DigitDisplay_DisplayOneSegment(digitsDisplay, 2, 15); // F
                Grove4DigitDisplay_DisplayOneSegment(digitsDisplay, 3, 14); // E
                unlock();

                delay(1500);
            }

            Dispatch(store, COFFEE_REQUEST_SHOWN, (void *)NULL);
        }
    }

    return NULL;
}

static void *Drive(void *data)
{
    int speed = 50;
    while (!terminationRequested)
    {
        if (MotorsEnabled(store))
        {
            lock();
            GroveI2CMotorDriver_SetMotorSpeeds(motorDriver, speed, speed);
            unlock();

            delay(1000);

            speed += 10;

            if (speed > 100)
            {
                lock();
                GroveI2CMotorDriver_StopMotors(motorDriver);
                unlock();
                speed = 50;

                Dispatch(store, MAX_MOTOR_SPEED_REACHED, (void *)NULL);
            }
        }
    }

    lock();
    GroveI2CMotorDriver_StopMotors(motorDriver);
    unlock();

    return NULL;
}

static void InitPeripheralsAndHandlers(void)
{
    struct sigaction action;
    memset(&action, 0, sizeof(struct sigaction));
    action.sa_handler = TerminationHandler;
    sigaction(SIGTERM, &action, NULL);

    state_t initialState = {
        .countdownEnabled = false, // should be true initially
        .coffeeRequestEnabled = false,
        .loadingIndicatorEnabled = true, // should be false initially
        .vibrationEnabled = false,
        .motorsEnabled = false,
        .temperature = UNKNOWN_VALUE,
        .humidity = UNKNOWN_VALUE};

    store = CreateStore(&initialState);

    GroveShield_Initialize(&i2cFd, 230400);

#ifdef I2C_SCAN
    i2cScanner(i2cScanner)
#endif

        GroveOledDisplay_Init(i2cFd, SH1107G);
    sht31 = GroveTempHumiSHT31_Open(i2cFd);
    vibrationMotor = GroveVibrationMotor_Open(TEMPLATE_VIBRATION_MOTOR);
    digitsDisplay = Grove4DigitDisplay_Open(TEMPLATE_4_DIGIT_DISPLAY_CLK, TEMPLATE_4_DIGIT_DISPLAY_DIO);
    motorDriver = GroveI2CMotorDriver_Open(i2cFd, DEFAULT_MOTOR_DRIVER_ADDRESS);

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

    pthread_t readTempHumiThread;
    pthread_create(&readTempHumiThread, NULL, ReadTempHumi, (void *)NULL);

    pthread_t showTempHumiThread;
    pthread_create(&showTempHumiThread, NULL, ShowTempHumi, (void *)NULL);

    pthread_t vibrateThread;
    pthread_create(&vibrateThread, NULL, Vibrate, (void *)NULL);

    pthread_t showCountdownThread;
    pthread_create(&showCountdownThread, NULL, ShowCountdown, (void *)NULL);

    pthread_t showLoadingIndicatorThread;
    pthread_create(&showLoadingIndicatorThread, NULL, ShowLoadingIndicator, (void *)NULL);

    pthread_t showCoffeeRequestThread;
    pthread_create(&showCoffeeRequestThread, NULL, ShowCoffeeRequest, (void *)NULL);

    pthread_t motorThread;
    pthread_create(&motorThread, NULL, Drive, (void *)NULL);

    pthread_join(blinkThread, NULL);
    pthread_join(readTempHumiThread, NULL);
    pthread_join(showTempHumiThread, NULL);
    pthread_join(vibrateThread, NULL);
    pthread_join(showCountdownThread, NULL);
    pthread_join(showCoffeeRequestThread, NULL);
    pthread_join(showLoadingIndicatorThread, NULL);
    pthread_join(motorThread, NULL);

    pthread_mutex_destroy(&mutex);

    DestroyStore(store);

    return 0;
}
