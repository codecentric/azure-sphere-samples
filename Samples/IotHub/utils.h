#pragma once

#include <applibs/log.h>
#include <applibs/wificonfig.h>
#include "../../Library/parson.h"

void DebugPrintCurrentlyConnectedWiFiNetwork(void);
bool GetBooleanValue(const JSON_Object *object, const char *name, const bool defaultValue);