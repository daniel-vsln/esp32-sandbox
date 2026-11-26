#include "WifiStatusView.h"

void showWifiState(ILed &led, WifiState state) {
  switch (state) {
  case WifiState::Idle:
    led.turnOff();
    led.turnBlinkOff();
    break;
  case WifiState::Connecting:
  case WifiState::Reconnecting:
    led.turnGreen();
    led.turnBlinkOn();
    break;
  case WifiState::Connected:
    led.turnGreen();
    led.turnBlinkOff();
    break;
  case WifiState::Failed:
    led.turnRed();
    led.turnBlinkOff();
    break;
  }
}
