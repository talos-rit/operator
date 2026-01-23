#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"
#include "log/log_config.hpp"

// Test group for LogConfig
TEST_GROUP(LogConfigTest) {
  LogConfig* logConfig;

  void setup() { logConfig = new LogConfig(); }

  void teardown() {
    delete logConfig;
    mock().clear();
  }
};

TEST(LogConfigTest, Initialization) {
  CHECK_TRUE(logConfig != NULL);

  // Test default log location
  const char* location = logConfig->GetLogLocation();
  STRCMP_EQUAL("/etc/talos/logs/operator.log", location);
}

TEST(LogConfigTest, SetLogLocation) {
  const char* new_location = "/var/log/custom.log";
  CHECK_EQUAL(0, logConfig->SetLogLocation(new_location));
  STRCMP_EQUAL(new_location, logConfig->GetLogLocation());
}
