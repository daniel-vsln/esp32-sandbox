#include "SerialCommandReader.h"

SerialCommandReader::SerialCommandReader(Stream &stream,
                                         ICommandProcessor &processor)
    : _stream{stream}, _processor{processor} {}

void SerialCommandReader::poll(unsigned long currentMillis) {
  if ((currentMillis - _prevMillis) > _period) {
    _prevMillis = currentMillis;
    if (_stream.available()) {
      _processor.process(_stream.readStringUntil('\n'));
    }
  }
}
