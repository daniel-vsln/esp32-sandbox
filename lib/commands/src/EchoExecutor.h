#pragma once

#include <ICommandExecutor.h>

class EchoExecutor : public ICommandExecutor {
  Stream &_stream;

public:
  explicit EchoExecutor(Stream &stream);
  void execute(std::vector<String> params) override;
};
