#pragma once

#include "WifiStateMachine.h"
#include <ILed.h>

void showWifiState(ILed &led, WifiState state);
