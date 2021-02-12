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

#include "../../MT3620_Grove_Shield_Library/Grove.h"
#include "../../MT3620_Grove_Shield_Library/Sensors/GroveOledDisplay96x96.h"

#include "../../Library/parson.h"
#include "../../Library/azure_iot_utilities.h"
#include "../../Library/led.h"
#include "../../Library/sleep.h"

#include "utils.h"
#include "device_twin.h"

static volatile sig_atomic_t terminationRequested = false;

static int i2cFd;
static leds_t leds;

static bool networkReady = false;
static bool connectedToIoTHub = false;

static bool hasGroveShield = false;
static bool hasDisplay = false;

static const char *pstrConnectionStatus = "Application started";

static void ShowDevicesOnScreen(int connectedDevices, int connectedNodeDevices, int connectedMainDevices)
{
  clearDisplay();
  setNormalDisplay();
  setVerticalMode();

  setTextXY(1, 0);
  putString("Main Node");

  setTextXY(4, 0);
  putString("Devices: ");
  putNumber(connectedDevices);

  setTextXY(6, 0);
  putString("Node Devices: ");
  putNumber(connectedNodeDevices);

  setTextXY(8, 0);
  putString("Main Devices: ");
  putNumber(connectedMainDevices);
}

static void SendEventMessage(const char *cstrEvent, const char *cstrMessage)
{
  JSON_Value *jsonRoot = json_value_init_object();
  json_object_set_string(json_object(jsonRoot), cstrEvent, cstrMessage);

  AzureIoT_SendJsonMessage(jsonRoot);

  json_value_free(jsonRoot);
}

static void IoTHubConnectionStatusUpdateHandler(bool connected, const char *statusText)
{
  connectedToIoTHub = connected;
  if (connectedToIoTHub)
  {
    SendEventMessage("connect", pstrConnectionStatus);
    DeviceTwinReportCapabilities(hasGroveShield, hasDisplay);

    Log_Debug("[IoTHubConnectionStatusChanged]: Connected.\n");
    pstrConnectionStatus = "connect";
  }
  else
  {
    Log_Debug("[IoTHubConnectionStatusChanged]: Disconnected.\n");
    pstrConnectionStatus = statusText;
  }
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

  DeviceTwinReportLedStates(&leds);

  int connectedDevices = -1;
  int connectedNodeDevices = -1;
  int connectedMainDevices = -1;

  if (json_object_dotget_value(desiredProperties, "connectedDevices") && hasGroveShield)
  {
    connectedDevices = (int)json_object_dotget_number(desiredProperties, "connectedDevices");
  }

  if (json_object_dotget_value(desiredProperties, "connectedNodeDevices") && hasGroveShield)
  {
    connectedNodeDevices = (int)json_object_dotget_number(desiredProperties, "connectedNodeDevices");
  }

  if (json_object_dotget_value(desiredProperties, "connectedMainDevices") && hasGroveShield)
  {
    connectedMainDevices = (int)json_object_dotget_number(desiredProperties, "connectedMainDevices");
  }

  if(hasGroveShield) {
    ShowDevicesOnScreen(connectedDevices, connectedNodeDevices, connectedMainDevices);
  }

  DeviceTwinReportConnectedDevices(connectedDevices, connectedNodeDevices, connectedMainDevices);
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

  OpenLeds(&leds);

  hasGroveShield = GroveShield_Initialize(&i2cFd, 230400) == 0;
  if (hasGroveShield)
  {
    hasDisplay = GroveOledDisplay_Init(i2cFd, SH1107G) == 0;
    if (hasDisplay) {
      ShowStartupScreen();
    }
  }
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