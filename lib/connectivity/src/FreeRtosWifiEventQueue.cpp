#include "FreeRtosWifiEventQueue.h"

bool FreeRtosWifiEventQueue::create() {
  if (_handle == nullptr)
    _handle = xQueueCreate(DEPTH, sizeof(WifiEvent));
  return _handle != nullptr;
}

bool FreeRtosWifiEventQueue::isCreated() const { return _handle != nullptr; }

bool FreeRtosWifiEventQueue::trySend(const WifiEvent &event) {
  return _handle != nullptr && xQueueSend(_handle, &event, 0) == pdTRUE;
}

bool FreeRtosWifiEventQueue::tryReceive(WifiEvent &event) {
  return _handle != nullptr && xQueueReceive(_handle, &event, 0) == pdTRUE;
}
