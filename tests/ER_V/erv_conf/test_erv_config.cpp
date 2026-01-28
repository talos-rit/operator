#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"
#include "erv_conf/erv_conf.hpp"

class TestERVConfig : public ERVConfig {
 public:
  ~TestERVConfig() = default;
  using ERVConfig::OverrideValue;  // Expose protected method for testing
};

TEST_GROUP(ERVConfigTest) {
  TestERVConfig* ervConfig;

  void setup() { ervConfig = new TestERVConfig(); }

  void teardown() {
    delete ervConfig;
    mock().clear();
  }
};

TEST(ERVConfigTest, Initialization) {
  CHECK_TRUE(ervConfig != NULL);

  // Test default Scorbot device path
  const char* dev_path = ervConfig->GetScorbotDevicePath();
  STRCMP_EQUAL("/dev/ttyUSB0", dev_path);
}

// TODO: Add more tests for ERVConfig functionality
IGNORE_TEST(ERVConfigTest, LoadDefaults) {
  // Change the device path first
  ervConfig->OverrideValue(ervConfig->AddKey("scorbot_device", "/dev/ttyUSB0"),
                           "/dev/ttyUSB1");
  STRCMP_EQUAL("/dev/ttyUSB0", ervConfig->GetScorbotDevicePath());

  // Now load defaults
  CHECK_EQUAL(0, ervConfig->LoadDefaults());

  // Check if the device path is reset to default
  const char* dev_path = ervConfig->GetScorbotDevicePath();
  STRCMP_EQUAL("/dev/ttyS0", dev_path);
}
