#pragma once

#include "ICommandExecutor.h"
#include "ICommandProcessor.h"
#include <map>

class CliCommandProcessor : public ICommandProcessor {
  std::map<String, ICommandExecutor &> _registeredCommands;

public:
  void registerCommand(const String &command, ICommandExecutor &executor);
  void unregisterCommand(const String &command);
  bool hasCommand(const String &command) const;
  bool process(const String &line) override;
};
