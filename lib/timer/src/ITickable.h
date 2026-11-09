#pragma once

#include <stdint.h>

struct ITickable {
  virtual void tick(uint32_t nowMs) = 0;

protected:
  ~ITickable() = default;
};
