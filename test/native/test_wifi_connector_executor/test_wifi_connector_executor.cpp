#include <ArduinoFake.h>
#include <WifiConnectorExecutor.h>
#include <unity.h>

using namespace fakeit;

struct FakeConnector : IWifiConnector {
  int connects = 0;
  int disconnects = 0;
  String ssid;
  String password;

  void connect(const String &s, const String &p) override {
    connects++;
    ssid = s;
    password = p;
  }
  void disconnect() override { disconnects++; }
};

static const char *USAGE =
    "Usage: wifi connect <ssid> <password> | wifi disconnect";
static String printed;

void setUp() {
  ArduinoFakeReset();
  printed = "";
  When(OverloadedMethod(ArduinoFake(Serial), println, size_t(const char *)))
      .AlwaysDo([](const char *text) {
        printed += text;
        return strlen(text);
      });
  When(OverloadedMethod(ArduinoFake(Serial), println, size_t(const String &)))
      .AlwaysDo([](const String &text) {
        printed += text;
        return text.length();
      });
}

void test_connect_passes_credentials() {
  FakeConnector connector;
  WifiConnectorExecutor executor{connector, Serial};

  executor.execute({"connect", "home", "secret"});

  TEST_ASSERT_EQUAL(1, connector.connects);
  TEST_ASSERT_EQUAL_STRING("home", connector.ssid.c_str());
  TEST_ASSERT_EQUAL_STRING("secret", connector.password.c_str());
  TEST_ASSERT_EQUAL_STRING("", printed.c_str());
}

void test_connect_without_credentials_prints_usage() {
  FakeConnector connector;
  WifiConnectorExecutor executor{connector, Serial};

  executor.execute({"connect"});

  TEST_ASSERT_EQUAL(0, connector.connects);
  TEST_ASSERT_EQUAL_STRING(USAGE, printed.c_str());
}

void test_connect_without_password_prints_usage() {
  FakeConnector connector;
  WifiConnectorExecutor executor{connector, Serial};

  executor.execute({"connect", "home"});

  TEST_ASSERT_EQUAL(0, connector.connects);
  TEST_ASSERT_EQUAL_STRING(USAGE, printed.c_str());
}

void test_disconnect_calls_connector() {
  FakeConnector connector;
  WifiConnectorExecutor executor{connector, Serial};

  executor.execute({"disconnect"});

  TEST_ASSERT_EQUAL(1, connector.disconnects);
  TEST_ASSERT_EQUAL(0, connector.connects);
  TEST_ASSERT_EQUAL_STRING("", printed.c_str());
}

void test_no_params_prints_usage() {
  FakeConnector connector;
  WifiConnectorExecutor executor{connector, Serial};

  executor.execute({});

  TEST_ASSERT_EQUAL(0, connector.connects);
  TEST_ASSERT_EQUAL(0, connector.disconnects);
  TEST_ASSERT_EQUAL_STRING(USAGE, printed.c_str());
}

void test_unknown_action_is_reported() {
  FakeConnector connector;
  WifiConnectorExecutor executor{connector, Serial};

  executor.execute({"scan"});

  TEST_ASSERT_EQUAL(0, connector.connects);
  TEST_ASSERT_EQUAL(0, connector.disconnects);
  TEST_ASSERT_EQUAL_STRING("Unknown wifi action: scan", printed.c_str());
}

void test_extra_params_are_ignored() {
  FakeConnector connector;
  WifiConnectorExecutor executor{connector, Serial};

  executor.execute({"connect", "home", "secret", "extra"});

  TEST_ASSERT_EQUAL(1, connector.connects);
  TEST_ASSERT_EQUAL_STRING("secret", connector.password.c_str());
}

void runTests() {
  UNITY_BEGIN();
  RUN_TEST(test_connect_passes_credentials);
  RUN_TEST(test_connect_without_credentials_prints_usage);
  RUN_TEST(test_connect_without_password_prints_usage);
  RUN_TEST(test_disconnect_calls_connector);
  RUN_TEST(test_no_params_prints_usage);
  RUN_TEST(test_unknown_action_is_reported);
  RUN_TEST(test_extra_params_are_ignored);
  UNITY_END();
}
