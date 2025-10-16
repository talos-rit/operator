
#include "socket/socket_conf.h"

#include <arpa/inet.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "conf/config.h"
#include "log/log.h"
#include "util/comm.h"

#define LOG_CONSOLE_THRESHOLD_THIS LOG_THRESHOLD_DEFAULT
#define LOG_FILE_THRESHOLD_THIS LOG_THRESHOLD_MAX

SocketConfig::SocketConfig() {
  address_idx =
      AddKey(SOCKET_CONF_BINDING_ADDR_KEY, SOCKET_CONF_BINDING_ADDR_DEFAULT);
  port_idx = AddKey(SOCKET_CONF_PORT_KEY, SOCKET_CONF_PORT_DEFAULT);
}

SocketConfig::~SocketConfig() {}

uint32_t SocketConfig::GetBindingAddress() {
  uint32_t val = 0;
  int ret = inet_pton(AF_INET, GetVal(address_idx), &val);
  if (1 == ret) return val;  // Nominal return

  // Error handling
  LOG_WARN("Failed to set user-defined binding address; using default");
  ret = inet_pton(AF_INET, SOCKET_CONF_BINDING_ADDR_DEFAULT, &val);
  if (1 == ret) return val;  // Default return

  // Default should always resolve; if you got here, you messed up.
  LOG_FATAL("Binding address could not be set to default; ending program");
  raise(SIGABRT);
  return 0;
}

int SocketConfig::SetBindingAddress(uint32_t addr) {
  char tmp[32];
  if (NULL == inet_ntop(AF_INET, &addr, &tmp[0], 32)) {
    LOG_WARN("Failed to override binding address; bad address");
    STD_FAIL;
  }

  return SetBindingAddress((const char *)&tmp);
}

int SocketConfig::SetBindingAddress(const char *addr) {
  if (OverrideValue(address_idx, addr)) {
    LOG_WARN("Failed to override binding address; bad address");
    STD_FAIL;
  }

  return 0;
}

uint16_t SocketConfig::GetPort() { return GetInt(port_idx); }

int SocketConfig::SetPort(uint16_t port) {
  return OverrideValue(port_idx, port);
}

int SocketConfig::LoadDefaults() { return 0; }