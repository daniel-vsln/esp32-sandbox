#pragma once

#include <Arduino.h>
#include <vector>

class ICommandExecutor {
public:
  virtual void execute(std::vector<String> params) = 0;
};
