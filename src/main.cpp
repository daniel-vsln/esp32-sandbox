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

TickManager<5> tickManager;
RgbPinOutput rgbOutput{48};
WifiLed wifiLed{rgbOutput};
WifiConnector wifiConnector{wifiLed};

WifiConnectorExecutor wifiConnectorExecutor{wifiConnector, Serial};
CliCommandProcessor commands;
SerialCommandReader console{Serial, commands};

uint8_t retryCount = 5;

void onWifiGenericEventCallback(WiFiEvent_t event, WiFiEventInfo_t info) {
  switch (event) {
  case ARDUINO_EVENT_WIFI_READY:
    wifiLed.turnWhite();
    // just ignore this one
    break;
  case ARDUINO_EVENT_WIFI_STA_START:
    Serial.println("[WiFi] Connection Started!");
    wifiLed.turnGreenBlinkOn();
    break;
  case ARDUINO_EVENT_WIFI_STA_CONNECTED:
    Serial.println("[WiFi] Connected!");
    retryCount = 5;
    break;
  case ARDUINO_EVENT_WIFI_STA_GOT_IP:
    Serial.print("[WiFi] Got IP address: ");
    Serial.println(WiFi.localIP());
    wifiLed.turnGreenSolidOn();
    break;
  default:
    break;
  }
}

void onWifiDisconnectedEventCallback(WiFiEvent_t event, WiFiEventInfo_t info) {
  switch (info.wifi_sta_disconnected.reason) {
  case WIFI_REASON_NO_AP_FOUND:
    if (retryCount > 0) {
      if (!wifiLed.isGreen() || !wifiLed.isBlinking()) {
        wifiLed.turnGreenBlinkOn();
      }
      Serial.println("[WiFi] No AP found. Retrying...");
      retryCount--;
    } else {
      Serial.println("[WiFi] No AP found. Max retries reached.");
      WiFi.disconnect();
      wifiLed.turnRedSolidOn();
    }
    break;
  case WIFI_REASON_4WAY_HANDSHAKE_TIMEOUT:
    if (retryCount > 0) {
      if (!wifiLed.isGreen() || !wifiLed.isBlinking()) {
        wifiLed.turnGreenBlinkOn();
      }

      Serial.println("[WiFi] Probably wrong password. Retrying...");
      retryCount--;
    } else {
      Serial.println("[WiFi] Probably wrong password. Max retries reached.");
      WiFi.disconnect();
      wifiLed.turnRedSolidOn();
    }
    break;
  case WIFI_REASON_AUTH_FAIL:
    Serial.println("[WiFi] Authentication failed. Check your password.");
    break;
  case WIFI_REASON_ASSOC_FAIL:
    Serial.println("[WiFi] Association failed. Check your network settings.");
    break;
  case WIFI_REASON_STA_LEAVING:
    // just ignore this one
    break;
  case WIFI_REASON_BEACON_TIMEOUT:
    Serial.println("[WiFi] Beacon timeout. Check your network settings.");
    break;
  default:
    Serial.println("[WiFi] Disconnected from WiFi. Reason: " +
                   String(info.wifi_sta_disconnected.reason));
    break;
  }
}

void setup() {
  Serial.begin(115200);
  // commands.registerCommand("wifi", wifiConnectorExecutor);

  tickManager.every(50, wifiLed, millis());
  // tickManager.every(50, wifiConnector, millis());

  // wifiConnector.connect(ssid, password);

  WiFi.onEvent(onWifiGenericEventCallback);
  WiFi.onEvent(onWifiDisconnectedEventCallback,
               ARDUINO_EVENT_WIFI_STA_DISCONNECTED);

  WiFi.begin(ssid, password);
}

void loop() {
  // console.poll(millis());
  tickManager.tick(millis());
}
