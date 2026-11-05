#include "WifiConnector.h"
#include <WiFi.h>

WifiConnector::WifiConnector(uint8_t rgbPin, const char *ssid,
                             const char *password)
    : _rgbPin{rgbPin} {
  WiFi.begin(ssid, password);
}
