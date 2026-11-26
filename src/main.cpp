#include "../secrets.cpp"
#include <Arduino.h>
#include <CliCommandProcessor.h>
#include <RgbPinOutput.h>
#include <SerialCommandReader.h>
#include <TickManager.h>
#include <WifiConnector.h>
#include <WifiConnectorExecutor.h>
#include <WifiLed.h>

TickManager<5> tickManager;
RgbPinOutput rgbOutput{48};
WifiLed wifiLed{rgbOutput};
WifiConnector wifiConnector{wifiLed};

WifiConnectorExecutor wifiConnectorExecutor{wifiConnector, Serial};
CliCommandProcessor commands;
SerialCommandReader console{Serial, commands};

void setup() {
  Serial.begin(115200);
  // commands.registerCommand("wifi", wifiConnectorExecutor);

  if (!wifiConnector.begin())
    Serial.println("[WiFi] Failed to start: could not create event queue");
  tickManager.every(50, wifiConnector, millis());
  tickManager.every(50, wifiLed, millis());

  wifiConnector.connect(ssid, password);
}

void loop() {
  // console.poll(millis());
  tickManager.tick(millis());
}
