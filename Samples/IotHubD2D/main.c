#include <stdbool.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#include <hw/template_appliance.h>

#include "applibs_versions.h"
#include <applibs/log.h>
#include <applibs/gpio.h>
#include <applibs/networking.h>
#include <applibs/wificonfig.h>
#include <applibs/powermanagement.h>

#include "utils.h"

#include "../../MT3620_Grove_Shield_Library/Grove.h"
#include "../../MT3620_Grove_Shield_Library/Sensors/GroveOledDisplay96x96.h"

#include "../../Library/parson.h"
#include "../../Library/azure_iot_utilities.h"
#include "../../Library/led.h"
#include "../../Library/sleep.h"

// #define MASTER_DEVICE

static volatile sig_atomic_t terminationRequested = false;

static int i2cFd;
static leds_t leds;

static bool networkReady = false;
static bool connectedToIoTHub = false;

static const char *pstrConnectionStatus = "Application started";

#ifdef MASTER_DEVICE
static void ReportConnectedDevices(int connectedDevices)
{
  if (connectedToIoTHub)
  {
    JSON_Value *root_value = json_value_init_object();
    JSON_Object *root_object = json_value_get_object(root_value);

    json_object_set_boolean(root_object, "connectedDevices", connectedDevices);

    AzureIoT_TwinReportStateJson(root_value);

    json_value_free(root_value);
  }
  else
  {
    Log_Debug("[ReportLedStates] not connected to IoT Hub: no led states reported.\n");
  }
}
#endif

static void ReportLedStates(void)
{
  if (connectedToIoTHub)
  {
    const led_channels_t ledChannels[] = {leds.one, leds.two, leds.three, leds.four, leds.wifi, leds.status};
    const char *ledJsonKeys[] = {"one", "two", "three", "four", "wifi", "status"};

    JSON_Value *root_value = json_value_init_object();
    JSON_Object *root_object = json_value_get_object(root_value);

    char jsonKeyBuffer[32];
    for (uint8_t i = 0; i < sizeof(ledChannels) / sizeof(led_channels_t); i++)
    {
      snprintf(jsonKeyBuffer, sizeof(jsonKeyBuffer), "leds.%s.red", ledJsonKeys[i]);
      json_object_dotset_boolean(root_object, jsonKeyBuffer, GetLedRedState(ledChannels[i]));

      snprintf(jsonKeyBuffer, sizeof(jsonKeyBuffer), "leds.%s.green", ledJsonKeys[i]);
      json_object_dotset_boolean(root_object, jsonKeyBuffer, GetLedGreenState(ledChannels[i]));

      snprintf(jsonKeyBuffer, sizeof(jsonKeyBuffer), "leds.%s.blue", ledJsonKeys[i]);
      json_object_dotset_boolean(root_object, jsonKeyBuffer, GetLedBlueState(ledChannels[i]));
    }

    AzureIoT_TwinReportStateJson(root_value);

    json_value_free(root_value);
  }
  else
  {
    Log_Debug("[ReportLedStates] not connected to IoT Hub: no led states reported.\n");
  }
}

static void SendEventMessage(const char *cstrEvent, const char *cstrMessage)
{
  if (connectedToIoTHub)
  {
    JSON_Value *jsonRoot = json_value_init_object();
    json_object_set_string(json_object(jsonRoot), cstrEvent, cstrMessage);

    AzureIoT_SendJsonMessage(jsonRoot);

    json_value_free(jsonRoot);
  }
  else
  {
    Log_Debug("[SendEventMessage] not connected to IoT Hub: no event sent.\n");
  }
}

static void IoTHubConnectionStatusUpdateHandler(bool connected, const char *statusText)
{
  connectedToIoTHub = connected;
  if (connectedToIoTHub)
  {
    Log_Debug("[IoTHubConnectionStatusChanged]: Connected.\n");
    SendEventMessage("connect", pstrConnectionStatus);
    pstrConnectionStatus = "connect";
  }
  else
  {
    Log_Debug("[IoTHubConnectionStatusChanged]: Disconnected.\n");
    pstrConnectionStatus = statusText;
  }
}

static void ShowDevicesOnScreen(int connectedDevices) {
  clearDisplay();
  setNormalDisplay();
  setVerticalMode();

  setTextXY(1, 0);
  putString("Master Node");
  
  setTextXY(3, 0);
  putString("Devices: ");
  putNumber(connectedDevices);
}

static void DeviceTwinUpdateHandler(JSON_Object *desiredProperties)
{
  Log_Debug("[IoTHubDeviceTwinUpdateReceived]: %s\n");

  SetLedIfExistsInJson(desiredProperties, leds.one, "one");
  SetLedIfExistsInJson(desiredProperties, leds.two, "two");
  SetLedIfExistsInJson(desiredProperties, leds.three, "three");
  SetLedIfExistsInJson(desiredProperties, leds.four, "four");
  SetLedIfExistsInJson(desiredProperties, leds.wifi, "wifi");
  SetLedIfExistsInJson(desiredProperties, leds.status, "status");

  ReportLedStates();

#ifdef MASTER_DEVICE
  if(json_object_dotget_value(desiredProperties, "connectedDevices") && i2cFd >= 0) 
  {
    int connectedDevices = (int) json_object_dotget_number(desiredProperties, "connectedDevices");
    ShowDevicesOnScreen(connectedDevices);

    ReportConnectedDevices(connectedDevices);
  }
#endif
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

static void NetworkLedUpdateHandler(void)
{
  Networking_IsNetworkingReady(&networkReady);
  SetLedState(leds.wifi, !networkReady, connectedToIoTHub, networkReady && !connectedToIoTHub);
}

static void InitIotHub(const char *dpsScopeId)
{
  AzureIoT_SetDPSScopeID(dpsScopeId);

  if (!AzureIoT_Initialize())
  {
    Log_Debug("ERROR: Cannot initialize Azure IoT Hub SDK.\n");
    exit(-1);
  }

  AzureIoT_SetDeviceTwinUpdateCallback(&DeviceTwinUpdateHandler);
  AzureIoT_SetConnectionStatusCallback(&IoTHubConnectionStatusUpdateHandler);
}

static void CloseIotHub(void)
{
  AzureIoT_DestroyClient();
  AzureIoT_Deinitialize();
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

#ifdef MASTER_DEVICE
  GroveShield_Initialize(&i2cFd, 230400);

  GroveOledDisplay_Init(i2cFd, SH1107G);
  clearDisplay();
#endif

  OpenLeds(&leds);
}

static void ClosePeripheralsAndHandlers(void)
{
  CloseLeds(&leds);
}

int main(int argc, char *argv[])
{
  if (argc < 2)
  {
    Log_Debug("ERROR: Please specify DPS scope id in app_manifest.json.\n");
    exit(-1);
  }

  InitIotHub(argv[1]);
  InitPeripheralsAndHandlers();

  DebugPrintCurrentlyConnectedWiFiNetwork();

  while (!terminationRequested)
  {
    NetworkLedUpdateHandler();
    AzureIoTPeriodicHandler();
    Sleep(1);
  }

  CloseIotHub();
  ClosePeripheralsAndHandlers();

  return 0;
}