#pragma once

#include <IWifiEventQueue.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

class FreeRtosWifiEventQueue : public IWifiEventQueue {
  QueueHandle_t _handle = nullptr;

public:
  // A connection attempt produces at most ~4 events (STA_CONNECTED, GOT_IP,
  // LOST_IP, DISCONNECTED) and loop() drains the queue every 50 ms, so even a
  // flapping link rarely queues more than a handful between two ticks.
  // 16 slots give ~4x headroom for 16 * 2 bytes of RAM; overflow is counted
  // by WifiEventInbox and reported from loop(); a lost event during an
  // attempt is recovered by the attempt timeout.
  static constexpr UBaseType_t DEPTH = 16;

  bool create();
  bool isCreated() const;

  bool trySend(const WifiEvent &event) override;
  bool tryReceive(WifiEvent &event) override;
};
