#pragma once

#include <ICommandExecutor.h>
#include <ILed.h>

class WifiLedExecutor : public ICommandExecutor {
  ILed &_led;
  Stream &_stream;

public:
  WifiLedExecutor(ILed &led, Stream &stream);
  void execute(std::vector<String> params) override;
};
