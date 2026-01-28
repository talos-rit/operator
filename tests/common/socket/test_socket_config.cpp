
#include <arpa/inet.h>

#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"
#include "socket/socket_conf.hpp"

// Test group for SocketConfig
TEST_GROUP(SocketConfigTestGroup) {
  SocketConfig* socketConfig;

  void setup() { socketConfig = new SocketConfig(); }

  void teardown() {
    delete socketConfig;
    mock().clear();
  }
};

TEST(SocketConfigTestGroup, SocketConfigInitialization) {
  CHECK_TRUE(socketConfig != NULL);

  // Test default port and default binding address
  CHECK_EQUAL(61616, socketConfig->GetPort());
  uint32_t expected_addr;
  inet_pton(AF_INET, SOCKET_CONF_BINDING_ADDR_DEFAULT, &expected_addr);
  CHECK_EQUAL(expected_addr, socketConfig->GetBindingAddress());
}

TEST(SocketConfigTestGroup, PortManagement) {
  // Test valid port setting
  CHECK_EQUAL(0, socketConfig->SetPort(8080));
  CHECK_EQUAL(8080, socketConfig->GetPort());

  // Test port retrieval after setting
  CHECK_EQUAL(8080, socketConfig->GetPort());
}

TEST(SocketConfigTestGroup, BindingAddressManagement) {
  // Test setting address by string
  CHECK_EQUAL(0, socketConfig->SetBindingAddress("192.168.1.100"));

  // Test getting address as uint32_t
  uint32_t addr = socketConfig->GetBindingAddress();
  CHECK_EQUAL(0xC0A80164, addr);  // 192.168.1.100 in network byte order

  // Test setting address by uint32_t
  CHECK_EQUAL(0, socketConfig->SetBindingAddress(0x7F000001));  // 127.0.0.1
  CHECK_EQUAL(0x7F000001, socketConfig->GetBindingAddress());
}

TEST(SocketConfigTestGroup, DefaultBindingAddress) {
  // The default should be 0.0.0.0 which is 0 in network byte order
  CHECK_EQUAL(0, socketConfig->GetBindingAddress());
}

TEST(SocketConfigTestGroup, LoadDefaults) {
  // Modify values first
  socketConfig->SetPort(9999);
  socketConfig->SetBindingAddress("10.0.0.1");

  // Load defaults should reset to original defaults
  CHECK_EQUAL(0, socketConfig->LoadDefaults());
  CHECK_EQUAL(61616, socketConfig->GetPort());
  CHECK_EQUAL(0, socketConfig->GetBindingAddress());  // 0.0.0.0
}