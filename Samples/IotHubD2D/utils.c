#include "utils.h"

#include <stdio.h>

void DebugPrintCurrentlyConnectedWiFiNetwork(void)
{
    WifiConfig_ConnectedNetwork network;
    int result = WifiConfig_GetCurrentNetwork(&network);
    if (result < 0) {
        Log_Debug("INFO: Not currently connected to a WiFi network.\n");
    } else {
        Log_Debug("INFO: Currently connected WiFi network: \n");
        Log_Debug("INFO: SSID \"%.*s\", BSSID %02x:%02x:%02x:%02x:%02x:%02x, Frequency %dMHz.\n",
                  network.ssidLength, network.ssid, network.bssid[0], network.bssid[1],
                  network.bssid[2], network.bssid[3], network.bssid[4], network.bssid[5],
                  network.frequencyMHz);
    }
}


bool GetBooleanValue(const JSON_Object *object, const char *name, const bool defaultValue)
{
  int result = json_object_dotget_boolean(object, name);
  return result == -1 ? defaultValue : result;
}


void SetLedIfExistsInJson(const JSON_Object *json, led_channels_t led, const char *ledName)
{
  char jsonKeyBuffer[32];

  snprintf(jsonKeyBuffer, sizeof(jsonKeyBuffer), "leds.%s.red", ledName);
  int ledRed = json_object_dotget_boolean(json, jsonKeyBuffer);

  snprintf(jsonKeyBuffer, sizeof(jsonKeyBuffer), "leds.%s.green", ledName);
  int ledGreen = json_object_dotget_boolean(json, jsonKeyBuffer);
  
  snprintf(jsonKeyBuffer, sizeof(jsonKeyBuffer), "leds.%s.blue", ledName);
  int ledBlue = json_object_dotget_boolean(json, jsonKeyBuffer);

  if(ledRed > -1){
      SetLedChannelFdState(led.red, ledRed);
  }
  
  if(ledGreen > -1){
      SetLedChannelFdState(led.green, ledGreen);
  }
  
  if(ledBlue > -1){
      SetLedChannelFdState(led.blue, ledBlue);
  }
}
