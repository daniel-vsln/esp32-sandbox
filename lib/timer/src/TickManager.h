#pragma once

#include "ITickable.h"
#include <array>
#include <stddef.h>

template <size_t N> class TickManager {
  struct Entry {
    ITickable *target;
    uint32_t interval;
    uint32_t lastTickMs;
  };

  std::array<Entry, N> entries{};
  size_t count = 0;

public:
  void tick(uint32_t nowMs) {
    for (size_t i = 0; i < count; ++i) {
      Entry &entry = entries[i];
      if (nowMs - entry.lastTickMs >= entry.interval) {
        entry.target->tick(nowMs);
        entry.lastTickMs = nowMs;
      }
    }
  }

  bool every(uint32_t interval, ITickable &t, uint32_t nowMs) {
    if (count >= N)
      return false;
    entries[count++] = {&t, interval, nowMs};
    return true;
  }
};
