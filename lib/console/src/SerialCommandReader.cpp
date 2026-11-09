#include "SerialCommandReader.h"

SerialCommandReader::SerialCommandReader(Stream &stream,
                                         ICommandProcessor &processor)
    : _stream{stream}, _processor{processor} {}

void SerialCommandReader::poll(unsigned long currentMillis) {
  if ((currentMillis - _prevMillis) > _period) {
    _prevMillis = currentMillis;
    if (_stream.available()) {
      String line = _stream.readStringUntil('\n');
      line.trim();
      if (line.length() > 0 && !_processor.process(line)) {
        _stream.print("Unknown command: ");
        _stream.println(line);
      }
    }
  }
}
