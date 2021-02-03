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

#include "led.h"

#define delay(ms) usleep(ms * 1000)

static volatile sig_atomic_t terminationRequested = false;

static leds_t leds;

static bool networkReady = false;
static bool connectedToIoTHub = false;

static void TerminationHandler(int signalNumber) {
  terminationRequested = true;
}

void NetworkLedUpdateHandler(void) {
  Networking_IsNetworkingReady(&networkReady);

  !bNetworkReady ?
  SetLedState(leds.wifi, true, false, false) :
  !connectedToIoTHub ? SetLedState(leds.wifi,
                                   false,
                                   true,
                                   false) : SetLedState(
      leds.wifi,
      false,
      false,
      true);
  if (bNetworkReady) {
    color = (connectedToIoTHub ? RgbLedUtility_Colors_Blue : RgbLedUtility_Colors_Green);
  } else {
    SetLedState(leds.wifi, true, false, false);
  }
}

static void InitPeripheralsAndHandlers(void) {
  struct sigaction action;
  memset(&action, 0, sizeof(struct sigaction));
  action.sa_handler = TerminationHandler;
  sigaction(SIGTERM, &action, NULL);

  OpenLeds(&leds);
}

int main(void) {
  InitPeripheralsAndHandlers();
  return 0;
}
