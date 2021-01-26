#include <errno.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "config.h"
#include "util.h"

#include <applibs/log.h>
#include <applibs/gpio.h>

#include "../../MT3620_Grove_Shield_Library/Grove.h"
#include "../../MT3620_Grove_Shield_Library/Sensors/GroveTempHumiSHT31.h"
#include "../../MT3620_Grove_Shield_Library/Sensors/GroveOledDisplay96x96.h"
#include "../../MT3620_Grove_Shield_Library/Sensors/GroveLEDButton.h"
#include "../../MT3620_Grove_Shield_Library/Sensors/GroveLightSensor.h"
#include "../../MT3620_Grove_Shield_Library/Sensors/GroveAD7992.h"
#include "../../MT3620_Grove_Shield_Library/Sensors/GroveRelay.h"

#include <hw/template_appliance.h>

typedef struct localState
{
    float temperature;
    float humidity;
    float analogLevel;
    GPIO_Value_Type ledButtonState;
    GPIO_Value_Type userButtonAState;
    GPIO_Value_Type userButtonBState;
    unsigned char displayContrast;
} localState_t;

static localState_t localState;
static int i2cFd;
static void *ledButtonHandler = NULL;
static void *temperatureHumudityHandler = NULL;
static void *analogSensorHandler = NULL;
static void *relayHandler = NULL;
static int userButtonAFd = -1;
static int userButtonBFd = -1;

static pthread_mutex_t i2cMutex = PTHREAD_MUTEX_INITIALIZER;

static pthread_cond_t buttonStateChangedCondition = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t buttonStateChangedMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t sensorsChangedCondition = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t sensorsChangedMutex = PTHREAD_MUTEX_INITIALIZER;

static int InitializePeripherals(void)
{
    // Register a SIGTERM handler for termination requests
    struct sigaction action;
    memset(&action, 0, sizeof(struct sigaction));
    action.sa_handler = TerminationHandler;
    sigaction(SIGTERM, &action, NULL);

    GroveShield_Initialize(&i2cFd, GROVE_SHIELD_BAUDRATE);

    // initialize temperature and humidity sensor
    temperatureHumudityHandler = GroveTempHumiSHT31_Open(i2cFd);
    if (temperatureHumudityHandler == NULL)
    {
        Log_Debug("Failed to initialize sht31 temperature and humidity handler\n");
        RequestTermination();
        return -1;
    }

    // initialize OLED with SH1107G driver (no gray scale)
    GroveOledDisplay_Init(i2cFd, SH1107G);

    // initialize led button with led on gpio 0, button on gpio 1
    ledButtonHandler = GroveLEDButton_Init(TEMPLATE_LED_BUTTON_SWITCH, TEMPLATE_LED_BUTTON_BLUE_LED);
    if (ledButtonHandler == NULL)
    {
        Log_Debug("Failed to initialize led button handler\n");
        RequestTermination();
        return -1;
    }

    // initialize light sensor (or potentiometer) on analog channel 0
    analogSensorHandler = GroveLightSensor_Init(i2cFd, ANALOG_READ_CHANNEL);
    if (analogSensorHandler == NULL)
    {
        Log_Debug("Failed to initialize analog sensor handler\n");
        RequestTermination();
        return -1;
    }

    // initialize relay or buzzer on GPIO 4
    relayHandler = GroveRelay_Open(TEMPLATE_RELAY);
    if (relayHandler == NULL)
    {
        Log_Debug("Failed to initialize relay handler\n");
        RequestTermination();
        return -1;
    }

    // initialize and clear display
    clearDisplay();
    setNormalDisplay();
    setVerticalMode();

    // initialize user buttons A and B (on sphere dec kit)
    userButtonAFd = GPIO_OpenAsInput(TEMPLATE_USER_BUTTON_A);
    if (userButtonAFd == -1)
    {
        Log_Debug("Failed to initialize user button A\n");
        RequestTermination();
        return -1;
    }

    // Open button B
    userButtonBFd = GPIO_OpenAsInput(TEMPLATE_USER_BUTTON_B);
    if (userButtonBFd == -1)
    {
        Log_Debug("Failed to initialize user button B\n");
        RequestTermination();
        return -1;
    }

    return 0;
}

static void displayString(unsigned char x, unsigned char y, char *text)
{
    pthread_mutex_lock(&i2cMutex);
    setTextXY(x, y);
    putString(text);
    pthread_mutex_unlock(&i2cMutex);
}

static void displaySetContrastLevel(unsigned char contrastLevel)
{
    localState.displayContrast = contrastLevel;

    pthread_mutex_lock(&i2cMutex);
    setContrastLevel(localState.displayContrast);
    pthread_mutex_unlock(&i2cMutex);
}

static void displayIncreaseContrastLevel(unsigned char delta)
{
    unsigned char newLevel;

    if (((int)localState.displayContrast) + delta > 255)
    {
        newLevel = 255;
    }
    else
    {
        newLevel = (unsigned char)(localState.displayContrast + delta);
    }

    displaySetContrastLevel(newLevel);
}

static void displayDecreaseContrastLevel(unsigned char delta)
{
    unsigned char newLevel;

    if (((int)localState.displayContrast) - delta < 0)
    {
        newLevel = 0;
    }
    else
    {
        newLevel = (unsigned char)(localState.displayContrast - delta);
    }

    displaySetContrastLevel(newLevel);
}

static float getRelayTemperatureThreshold(void)
{
    return UPDATE_RELAY_BY_TEMPERATURE_THRESHOLD_MIN + ((UPDATE_RELAY_BY_TEMPERATURE_THRESHOLD_MAX - UPDATE_RELAY_BY_TEMPERATURE_THRESHOLD_MIN) * localState.analogLevel);
}

static void *UpdateDisplayThread(void *arguments)
{
    Log_Debug("Starting thread display\n");

    pthread_mutex_lock(&sensorsChangedMutex);

    while (!IsTerminationRequested())
    {
        pthread_cond_wait(&sensorsChangedCondition, &sensorsChangedMutex);

        Log_Debug("Updating display...\n");

        char buffer[16];

        // temperature
        memset(buffer, 0, sizeof(buffer));
        sprintf(buffer, "T: %2.2f C", localState.temperature);
        Log_Debug("%s\n", buffer);
        displayString(0, 0, buffer);

        //humidity
        memset(buffer, 0, sizeof(buffer));
        sprintf(buffer, "H: %2.2f %%", localState.humidity);
        Log_Debug("%s\n", buffer);
        displayString(1, 0, buffer);

        // analog level
        memset(buffer, 0, sizeof(buffer));
        sprintf(buffer, "A: %3.0f %%", localState.analogLevel * 100.0);
        Log_Debug("%s\n", buffer);
        displayString(2, 0, buffer);

        // temperature threshold current
        displayString(7, 0, "Temp. threshold:");
        memset(buffer, 0, sizeof(buffer));
        sprintf(buffer, "%2.1f C", getRelayTemperatureThreshold());
        displayString(8, 0, buffer);

        // display contrast
        memset(buffer, 0, sizeof(buffer));
        sprintf(buffer, "Contrast: %3.0f %%", ((localState.displayContrast / 255.0f) * 100.0f));
        Log_Debug("%s\n", buffer);
        displayString(10, 0, buffer);

        usleep(DISPLAY_THREAD_MIN_UPDATE_INTERVAL_MS * 1000);
    }

    clearDisplay();

    pthread_mutex_unlock(&sensorsChangedMutex);

    return NULL;
}

static void *UpdateSensorsThread(void *arguments)
{
    Log_Debug("Starting thread sensors\n");

    while (!IsTerminationRequested())
    {
        // get analog value and set display contrast using that value
        pthread_mutex_lock(&i2cMutex);
        localState.analogLevel = GroveLightSensor_Read(analogSensorHandler);
        pthread_mutex_unlock(&i2cMutex);

        // get and display temperature and humidity
        pthread_mutex_lock(&i2cMutex);
        GroveTempHumiSHT31_Read(temperatureHumudityHandler);
        pthread_mutex_unlock(&i2cMutex);
        localState.temperature = GroveTempHumiSHT31_GetTemperature(temperatureHumudityHandler);
        localState.humidity = GroveTempHumiSHT31_GetHumidity(temperatureHumudityHandler);

        pthread_cond_broadcast(&sensorsChangedCondition);

        usleep(SENSORS_THREAD_UPDATE_INTERVAL_MS * 1000);
    }

    return NULL;
}

static void *UpdateButtonStateThread(void *arguments)
{
    Log_Debug("Starting thread button state\n");

    GPIO_Value_Type btnLed;
    GPIO_Value_Type btnUserA;
    GPIO_Value_Type btnUserB;

    bool initialized = false;

    while (!IsTerminationRequested())
    {
        //Log_Debug("Getting button state...\n");

        // get button states
        btnLed = GroveLEDButton_GetBtnState(ledButtonHandler);
        GPIO_GetValue(userButtonAFd, &btnUserA);
        GPIO_GetValue(userButtonBFd, &btnUserB);

        if ((localState.userButtonAState != btnUserA) || (localState.userButtonBState != btnUserB) || (localState.ledButtonState != btnLed) || (!initialized))
        {
            if (!btnUserA || !btnUserB || !btnLed)
            {
                Log_Debug("Button pressed.\n");
                GroveLEDButton_LedOn(ledButtonHandler);
                displayString(4, 0, "Button pressed. ");
            }
            else
            {
                Log_Debug("Button released.\n");
                GroveLEDButton_LedOff(ledButtonHandler);
                displayString(4, 0, "Button released.");
            }

            localState.ledButtonState = btnLed;
            localState.userButtonAState = btnUserA;
            localState.userButtonBState = btnUserB;

            initialized = true;

            pthread_cond_signal(&buttonStateChangedCondition);
        }

        usleep(BUTTON_THREAD_UPDATE_INTERVAL_MS * 1000);
    }

    return NULL;
}

static void *ButtonTriggeredThread(void *arguments)
{
    Log_Debug("Starting thread relay\n");

    pthread_mutex_lock(&buttonStateChangedMutex);

    while (!IsTerminationRequested())
    {
        pthread_cond_wait(&buttonStateChangedCondition, &buttonStateChangedMutex);

        // react to button trigger events here:

        if (!localState.ledButtonState)
        {
            Log_Debug("LED Button pressed\n");
        }

        if (!localState.userButtonAState)
        {
            Log_Debug("Increase contrast\n");

            displayIncreaseContrastLevel(32);
        }

        if (!localState.userButtonBState)
        {
            Log_Debug("Decrease contrast\n");

            displayDecreaseContrastLevel(32);
        }
    }

    pthread_mutex_unlock(&buttonStateChangedMutex);

    return NULL;
}

static void *UpdateRelayByTemperatureThread(void *arguments)
{
    Log_Debug("Starting thread relay\n");

    pthread_mutex_lock(&sensorsChangedMutex);

    while (!IsTerminationRequested())
    {
        pthread_cond_wait(&sensorsChangedCondition, &sensorsChangedMutex);

        Log_Debug("Updating relay state by temperature...\n");

        if (localState.temperature >= getRelayTemperatureThreshold())
        {
            GroveRelay_On(relayHandler);
            Log_Debug("Relay on\n");
            displayString(5, 0, "Relay on.  ");
        }
        else
        {
            GroveRelay_Off(relayHandler);
            Log_Debug("Relay off\n");
            displayString(5, 0, "Relay off.");
        }
    }

    pthread_mutex_unlock(&sensorsChangedMutex);

    return NULL;
}

int main(int argc, char *argv[])
{
    Log_Debug("Application starting\n");

    pthread_t updateButtonStateThreadHandler;
    pthread_t updateSensorsThreadHandler;
    pthread_t updateDisplayThreadHandler;
    pthread_t updateRelayByTemperatureThreadHandler;
    pthread_t buttonTriggeredThreadHandler;

    // initialize local state
    localState.analogLevel = 0;
    localState.displayContrast = 128;
    localState.humidity = 0;
    localState.temperature = 0;
    localState.ledButtonState = 1;
    localState.userButtonAState = 1;
    localState.userButtonBState = 1;

    // intialize peripherals
    InitializePeripherals();

    int result_code = pthread_create(&updateButtonStateThreadHandler, NULL, UpdateButtonStateThread, NULL);
    if (result_code)
    {
        Log_Debug("Failed to create thread update button state\n");
    }
    result_code = pthread_create(&updateSensorsThreadHandler, NULL, UpdateSensorsThread, NULL);
    if (result_code)
    {
        Log_Debug("Failed to create thread update sensors\n");
    }
    result_code = pthread_create(&updateDisplayThreadHandler, NULL, UpdateDisplayThread, NULL);
    if (result_code)
    {
        Log_Debug("Failed to create thread update display\n");
    }

    result_code = pthread_create(&updateRelayByTemperatureThreadHandler, NULL, UpdateRelayByTemperatureThread, NULL);
    if (result_code)
    {
        Log_Debug("Failed to create thread update relay by temperature\n");
    }

    result_code = pthread_create(&buttonTriggeredThreadHandler, NULL, ButtonTriggeredThread, NULL);
    if (result_code)
    {
        Log_Debug("Failed to create thread button triggered\n");
    }

    pthread_join(updateButtonStateThreadHandler, NULL);
    pthread_join(updateSensorsThreadHandler, NULL);
    pthread_join(updateDisplayThreadHandler, NULL);
    pthread_join(updateRelayByTemperatureThreadHandler, NULL);
    pthread_join(buttonTriggeredThreadHandler, NULL);

    Log_Debug("Application exiting\n");

    pthread_mutex_destroy(&i2cMutex);
    pthread_mutex_destroy(&buttonStateChangedMutex);
    pthread_mutex_destroy(&sensorsChangedMutex);

    pthread_cond_destroy(&buttonStateChangedCondition);
    pthread_cond_destroy(&sensorsChangedCondition);

    return 0;
}
