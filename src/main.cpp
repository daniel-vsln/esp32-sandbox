#include "../secrets.cpp"
#include <Arduino.h>
#include <RgbPinOutput.h>
#include <TickManager.h>
#include <WifiConnector.h>
#include <WifiLed.h>

TickManager<5> tickManager;
RgbPinOutput rgbOutput{RGB_BUILTIN};
WifiLed wifiLed{rgbOutput};
WifiConnector wifiConnector{wifiLed};

void setup() {
  Serial.begin(115200);

  if (!wifiConnector.begin())
    Serial.println("[WiFi] Failed to start: could not create event queue");
  tickManager.every(50, wifiConnector, millis());
  tickManager.every(50, wifiLed, millis());

  wifiConnector.connect(ssid, password);
}

void loop() { tickManager.tick(millis()); }
