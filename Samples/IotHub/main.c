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

#include "../../Library/parson.h"
#include "../../Library/azure_iot_utilities.h"
#include "../../Library/led.h"
#include "../../Library/sleep.h"

static volatile sig_atomic_t terminationRequested = false;

static int rebootDeviceDelay = -1;

static leds_t leds;

static bool networkReady = false;
static bool connectedToIoTHub = false;

static const char *pstrConnectionStatus = "Application started";

HTTP_STATUS_CODE RebootMethod(JSON_Value *jsonParameters, JSON_Value **jsonResponseAddress)
{
  Log_Debug("[RebootMethod]: Invoked.\n");

  HTTP_STATUS_CODE result = HTTP_BAD_REQUEST;
  JSON_Value *jsonResponse = json_value_init_object();
  JSON_Object *jsonObject = json_value_get_object(jsonResponse);

  if (jsonParameters != NULL)
  {
    JSON_Object *jsonRootObject = json_value_get_object(jsonParameters);
    rebootDeviceDelay = (int) json_object_dotget_number(jsonRootObject, "delay");

    if (rebootDeviceDelay > 0)
    {
      Log_Debug("[RebootMethod]: rebooting device in %d seconds.\n", rebootDeviceDelay);
      json_object_set_boolean(jsonObject, "success", true);
      result = HTTP_OK;
    }
    else
    {
      Log_Debug("[RebootMethod]: not rebooting device because no delay was given.\n");
      json_object_set_boolean(jsonObject, "success", false);
    }
  }

  *jsonResponseAddress = jsonResponse;
  return result;
}

static const MethodRegistration RebootDeviceMethodHandler =
    {.MethodName = "rebootMethod", .MethodHandler = &RebootMethod};

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

static void SetLedStateReported(led_channels_t channel, bool red, bool green, bool blue)
{
  SetLedState(channel, red, green, blue);
  ReportLedStates();
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

static void IoTHubMessageReceiveHandler(const char *messageBody)
{
  Log_Debug("[IoTHubMessageReceived]: %s\n", messageBody);
  SetLedStateReported(leds.one, GetLedRedState(leds.one), false, false);
}

static void DeviceTwinUpdateHandler(JSON_Object *desiredProperties)
{
  Log_Debug("[IoTHubDeviceTwinUpdateReceived]: %s\n");

  bool led1Red = GetBooleanValue(desiredProperties, "leds.one.red", false);
  bool led1Green = GetBooleanValue(desiredProperties, "leds.one.green", false);
  bool led1Blue = GetBooleanValue(desiredProperties, "leds.one.blue", false);

  bool led2Red = GetBooleanValue(desiredProperties, "leds.two.red", false);
  bool led2Green = GetBooleanValue(desiredProperties, "leds.two.green", false);
  bool led2Blue = GetBooleanValue(desiredProperties, "leds.two.blue", false);

  bool led3Red = GetBooleanValue(desiredProperties, "leds.three.red", false);
  bool led3Green = GetBooleanValue(desiredProperties, "leds.three.green", false);
  bool led3Blue = GetBooleanValue(desiredProperties, "leds.three.blue", false);

  bool led4Red = GetBooleanValue(desiredProperties, "leds.four.red", false);
  bool led4Green = GetBooleanValue(desiredProperties, "leds.four.green", false);
  bool led4Blue = GetBooleanValue(desiredProperties, "leds.four.blue", false);

  bool ledWifiRed = GetBooleanValue(desiredProperties, "leds.wifi.red", false);
  bool ledWifiGreen = GetBooleanValue(desiredProperties, "leds.wifi.green", false);
  bool ledWifiBlue = GetBooleanValue(desiredProperties, "leds.wifi.blue", false);

  bool ledStatusRed = GetBooleanValue(desiredProperties, "leds.status.red", false);
  bool ledStatusGreen = GetBooleanValue(desiredProperties, "leds.status.green", false);
  bool ledStatusBlue = GetBooleanValue(desiredProperties, "leds.status.blue", false);

  SetLedState(leds.one, led1Red, led1Green, led1Blue);
  SetLedState(leds.two, led2Red, led2Green, led2Blue);
  SetLedState(leds.three, led3Red, led3Green, led3Blue);
  SetLedState(leds.four, led4Red, led4Green, led4Blue);
  SetLedState(leds.wifi, ledWifiRed, ledWifiGreen, ledWifiBlue);
  SetLedState(leds.status, ledStatusRed, ledStatusGreen, ledStatusBlue);

  ReportLedStates();
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

  AzureIoT_SetMessageReceivedCallback(&IoTHubMessageReceiveHandler);
  AzureIoT_SetDeviceTwinUpdateCallback(&DeviceTwinUpdateHandler);
  AzureIoT_RegisterDirectMethodHandlers(&RebootDeviceMethodHandler);
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

  while (!terminationRequested && rebootDeviceDelay <= 0)
  {
    NetworkLedUpdateHandler();
    AzureIoTPeriodicHandler();
    Sleep(1);
  }

  CloseIotHub();
  ClosePeripheralsAndHandlers();

  if (rebootDeviceDelay > 0)
  {
    Sleep((unsigned)(rebootDeviceDelay * 1000));
    PowerManagement_ForceSystemReboot();
  }

  return 0;
}