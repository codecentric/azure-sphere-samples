#include <stdbool.h>
#include "../../Library/led.h"

void DeviceTwinReportCapabilities(bool hasGroveShield, bool hasDisplay);

void DeviceTwinReportConnectedDevices(int connectedDevices, int nodeDevices, int mainDevices);

void DeviceTwinReportLedStates(leds_t *leds);