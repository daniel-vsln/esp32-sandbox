#include "CommandLine.h"

ParsedCommand parseCommandLine(const String &line) {
  String input = line;
  input.trim();

  ParsedCommand parsed;
  int length = input.length();
  int start = 0;

  while (start < length) {
    int end = input.indexOf(' ', start);
    if (end == -1) {
      end = length;
    }

    if (end > start) {
      String token = input.substring(start, end);
      if (parsed.name.length() == 0) {
        parsed.name = token;
      } else {
        parsed.params.push_back(token);
      }
    }

    start = end + 1;
  }

  return parsed;
}
