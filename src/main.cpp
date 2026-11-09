#include "../secrets.cpp"
#include <Arduino.h>
#include <CliCommandProcessor.h>
#include <RgbPinOutput.h>
#include <SerialCommandReader.h>
#include <TickManager.h>
#include <WiFi.h>
#include <WifiConnector.h>
#include <WifiConnectorExecutor.h>
#include <WifiLed.h>

void setupWiFi() {
  Serial.begin(115200);
  delay(10);

  pinMode(48, INPUT_PULLUP);
}

TickManager<5> tickManager;
RgbPinOutput rgbOutput{48};
WifiLed wifiLed{rgbOutput};
WifiConnector wifiConnector{wifiLed};

WifiConnectorExecutor wifiConnectorExecutor{wifiConnector, Serial};
CliCommandProcessor commands;
SerialCommandReader console{Serial, commands};

void setup() {
  Serial.begin(115200);
  commands.registerCommand("wifi", wifiConnectorExecutor);

  tickManager.every(50, wifiLed, millis());
  tickManager.every(50, wifiConnector, millis());
}

void loop() {
  console.poll(millis());
  tickManager.tick(millis());
}
