#pragma once

#include "IWifiEventQueue.h"
#include <atomic>
#include <stdint.h>

class WifiEventInbox {
  IWifiEventQueue &_queue;
  std::atomic<uint32_t> _dropped{0};

public:
  explicit WifiEventInbox(IWifiEventQueue &queue);

  void post(const WifiEvent &event);
  bool take(WifiEvent &event);
  uint32_t takeDroppedCount();
};
