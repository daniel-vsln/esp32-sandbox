#pragma once

#include <ICommandExecutor.h>
#include <IWifiConnector.h>

class WifiConnectorExecutor : public ICommandExecutor {
  IWifiConnector &_connector;
  Stream &_stream;

  void printUsage();

public:
  WifiConnectorExecutor(IWifiConnector &connector, Stream &stream);
  void execute(std::vector<String> params) override;
};
