#include "WifiEventInbox.h"

WifiEventInbox::WifiEventInbox(IWifiEventQueue &queue) : _queue{queue} {}

void WifiEventInbox::post(const WifiEvent &event) {
  if (!_queue.trySend(event))
    _dropped.fetch_add(1, std::memory_order_relaxed);
}

bool WifiEventInbox::take(WifiEvent &event) { return _queue.tryReceive(event); }

uint32_t WifiEventInbox::takeDroppedCount() {
  return _dropped.exchange(0, std::memory_order_relaxed);
}
