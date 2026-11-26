#pragma once

#include "WifiStateMachine.h"

struct IWifiEventQueue {
  virtual bool trySend(const WifiEvent &event) = 0;
  virtual bool tryReceive(WifiEvent &event) = 0;

protected:
  ~IWifiEventQueue() = default;
};
