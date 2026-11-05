#include "CliCommandProcessor.h"
#include "CommandLine.h"

void CliCommandProcessor::registerCommand(const String &command,
                                          ICommandExecutor &executor) {
  _registeredCommands.insert({command, executor});
}

void CliCommandProcessor::unregisterCommand(const String &command) {
  _registeredCommands.erase(command);
}

bool CliCommandProcessor::hasCommand(const String &command) const {
  return _registeredCommands.find(command) != _registeredCommands.end();
}

bool CliCommandProcessor::process(const String &line) {
  auto parsed = parseCommandLine(line);

  auto executor = _registeredCommands.find(parsed.name);
  if (executor == _registeredCommands.end()) {
    return false;
  }

  executor->second.execute(parsed.params);
  return true;
}
