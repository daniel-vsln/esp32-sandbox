#include "EchoExecutor.h"

EchoExecutor::EchoExecutor(Stream &stream) : _stream{stream} {}

void EchoExecutor::execute(std::vector<String> params) {
  String text;
  for (const String &param : params) {
    if (text.length() > 0) {
      text += " ";
    }
    text += param;
  }

  _stream.print(text + "... " + text + "... " + text + "...");
}
