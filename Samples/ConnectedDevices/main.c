#include <signal.h>
#include <stdlib.h>
#include <stdbool.h>

#include <applibs/log.h>
#include <applibs/networking.h>

#include "../../Library/azure_iot_utilities.h"
#include "../../Library/sleep.h"
#include "../../Library/led.h"
#include "../../Library/exit.h"
#include "../../Library/parson.h"

#include "../../MT3620_Grove_Shield_Library/Grove.h"
#include "../../MT3620_Grove_Shield_Library/Sensors/GroveOledDisplay96x96.h"

static volatile sig_atomic_t terminationRequested = false;

static leds_t leds;

static bool networkReady = false;
static bool connectedToIoTHub = false;

static void IoTHubConnectionStatusUpdateHandler(bool connected, const char *statusText)
{
    connectedToIoTHub = connected;
}

static uint8_t *ImageToByteArray(const JSON_Array *image, size_t *imageSize)
{
    size_t count = json_array_get_count(image);
    *imageSize = count * sizeof(uint8_t);
    uint8_t *imageData = malloc(*imageSize);
    for (size_t index = 0; index < count; index++)
    {
        imageData[index] = (uint8_t)json_array_get_number(image, index);
    }
    return imageData;
}

static void DeviceTwinUpdateHandler(JSON_Object *desiredProperties)
{
    Log_Debug("[IoTHubDeviceTwinUpdateReceived]: %s\n");

    const char *text = json_object_dotget_string(desiredProperties, "text");
    const JSON_Array *image = json_object_dotget_array(desiredProperties, "image");

    if (image != NULL)
    {
        clearDisplay();

        size_t imageSize;
        uint8_t *imageData = ImageToByteArray(image, &imageSize);
        drawBitmap(imageData, (int)imageSize);
        free(imageData);

        if (connectedToIoTHub)
        {
            JSON_Value *rootValue = json_value_init_object();
            JSON_Object *rootObject = json_value_get_object(rootValue);
            json_object_set_value(rootObject, "image", json_value_deep_copy(json_object_get_value(desiredProperties, "image")));
            AzureIoT_TwinReportStateJson(rootValue);
        }
    }
    else if (text != NULL)
    {
        clearDisplay();
        setNormalDisplay();
        setVerticalMode();

        setTextXY(1, 0);
        putString(text);

        if (connectedToIoTHub)
        {
            JSON_Value *rootValue = json_value_init_object();
            JSON_Object *rootObject = json_value_get_object(rootValue);
            json_object_set_string(rootObject, "text", text);

            AzureIoT_TwinReportStateJson(rootValue);

            free((char*)text);
        }
    }
}

static void InitIotHub(const char *dpsScopeId)
{
    AzureIoT_SetDPSScopeID(dpsScopeId);

    if (!AzureIoT_Initialize())
    {
        Log_Debug("ERROR: Cannot initialize Azure IoT Hub SDK.\n");
        exit(ExitCode_Initialization_Failed);
    }

    //    AzureIoT_SetMessageReceivedCallback(&IoTHubMessageReceiveHandler);
    AzureIoT_SetDeviceTwinUpdateCallback(&DeviceTwinUpdateHandler);
    //    AzureIoT_RegisterDirectMethodHandlers(&RebootDeviceMethodHandler);
    AzureIoT_SetConnectionStatusCallback(&IoTHubConnectionStatusUpdateHandler);
}

static void NetworkLedUpdateHandler(void)
{
    Networking_IsNetworkingReady(&networkReady);
    SetLedState(leds.wifi, !networkReady, connectedToIoTHub, networkReady && !connectedToIoTHub);
}

static void TerminationHandler(int signalNumber)
{
    terminationRequested = true;
}

static void InitPeripheralsAndHandlers(void)
{
    struct sigaction action;
    memset(&action, 0, sizeof(struct sigaction));
    action.sa_handler = TerminationHandler;
    sigaction(SIGTERM, &action, NULL);

    static int i2cFd;
    GroveShield_Initialize(&i2cFd, 230400);
    GroveOledDisplay_Init(i2cFd, SH1107G);

    OpenLeds(&leds);
}

static void CloseIotHub(void)
{
    AzureIoT_DestroyClient();
    AzureIoT_Deinitialize();
}

static void ClosePeripheralsAndHandlers(void)
{
    CloseLeds(&leds);
}

static void AzureIoTPeriodicHandler(void)
{
    if (!networkReady)
    {
        return;
    }

    if (AzureIoT_SetupClient())
    {
        AzureIoT_DoPeriodicTasks();
    }
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        Log_Debug("ERROR: Please specify DPS scope id in app_manifest.json.\n");
        exit(ExitCode_Initialization_Failed);
    }

    InitIotHub(argv[1]);
    InitPeripheralsAndHandlers();

    while (!terminationRequested)
    {
        NetworkLedUpdateHandler();
        AzureIoTPeriodicHandler();
        Sleep(100);
    }

    CloseIotHub();
    ClosePeripheralsAndHandlers();

    return ExitCode_Success;
}