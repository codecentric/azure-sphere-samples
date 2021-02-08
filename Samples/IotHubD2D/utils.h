#pragma once

#include <applibs/log.h>
#include <applibs/wificonfig.h>

#include "../../Library/parson.h"
#include "../../Library/led.h"


void DebugPrintCurrentlyConnectedWiFiNetwork(void);

void showStartupScreen(void);

bool GetBooleanValue(const JSON_Object *object, const char *name, const bool defaultValue);

void SetLedIfExistsInJson(const JSON_Object *json, led_channels_t led, const char *ledName);