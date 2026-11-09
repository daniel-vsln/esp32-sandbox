#include "WifiConnector.h"
#include <WiFi.h>

WifiConnector::WifiConnector(WifiLed &led) : _led{&led} {}

void WifiConnector::tick(uint32_t) {
  wl_status_t status = WiFi.status();

  if (status == WL_CONNECTED) {
    if (_state != CONNECTED) {
      Serial.println("[WiFi] Connected to WiFi!");
      _state = CONNECTED;
      _led->turnGreen();
      _led->turnBlinkOff();
    }
  } else if (status == WL_DISCONNECTED) {
    if (_state == CONNECTING) {
      Serial.println(
          "[WiFi] Disconnected from WiFi. Attempting to reconnect...");
      _state = RECONNECT;
      connect(_ssid, _password);
    }
  }
}

void WifiConnector::connect(const String &ssid, const String &password) {
  _ssid = ssid;
  _password = password;

  Serial.print("[WiFi] Connecting to ");
  Serial.println(_ssid);

  WiFi.begin(_ssid.c_str(), _password.c_str());
  _led->turnBlinkOn();
  _led->turnGreen();
  _state = CONNECTING;
}

void WifiConnector::disconnect() { WiFi.disconnect(); }
