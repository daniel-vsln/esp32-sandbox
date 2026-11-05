#pragma once

#include <Arduino.h>
#include <vector>

struct ParsedCommand {
  String name;
  std::vector<String> params;
};

ParsedCommand parseCommandLine(const String &line);
