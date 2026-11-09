#include "WifiLedExecutor.h"

WifiLedExecutor::WifiLedExecutor(ILed &led, Stream &stream)
    : _led{led}, _stream{stream} {}

void WifiLedExecutor::execute(std::vector<String> params) {
  if (params.empty()) {
    _stream.println("Usage: led <color>");
    return;
  }

  const String &action = params[0];
  if (action == "green") {
    _led.turnGreen();
  } else if (action == "red") {
    _led.turnRed();
  } else if (action == "white") {
    _led.turnWhite();
  } else if (action == "off") {
    _led.turnOff();
  } else if (action == "blink_on") {
    _led.turnBlinkOn();
  } else if (action == "blink_off") {
    _led.turnBlinkOff();
  } else {
    _stream.println("Unknown color: " + action);
  }
}
