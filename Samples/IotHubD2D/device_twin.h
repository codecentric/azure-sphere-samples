#include <stdbool.h>
#include "../../Library/led.h"

void DeviceTwinReportCapabilities(bool hasGroveShield, bool hasDisplay, bool hasTempAndHumidity);

void DeviceTwinReportConnectedDevices(int connectedDevices, int nodeDevices, int mainDevices);

void DeviceTwinReportLedStates(leds_t *leds);