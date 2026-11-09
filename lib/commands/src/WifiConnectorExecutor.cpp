#include "WifiConnectorExecutor.h"

WifiConnectorExecutor::WifiConnectorExecutor(IWifiConnector &connector,
                                             Stream &stream)
    : _connector{connector}, _stream{stream} {}

void WifiConnectorExecutor::printUsage() {
  _stream.println("Usage: wifi connect <ssid> <password> | wifi disconnect");
}

void WifiConnectorExecutor::execute(std::vector<String> params) {
  if (params.empty()) {
    printUsage();
    return;
  }

  const String &action = params[0];
  if (action == "connect") {
    if (params.size() < 3) {
      printUsage();
      return;
    }
    _connector.connect(params[1], params[2]);
  } else if (action == "disconnect") {
    _connector.disconnect();
  } else {
    _stream.println("Unknown wifi action: " + action);
  }
}
