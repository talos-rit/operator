#pragma once

#include <stdint.h>

#include "conf/config.hpp"

#define SOCKET_CONF_BINDING_ADDR_KEY "socket_address"
#define SOCKET_CONF_BINDING_ADDR_DEFAULT "0.0.0.0"
#define SOCKET_CONF_PORT_KEY "socket_port"
#define SOCKET_CONF_PORT_DEFAULT 61616

class SocketConfig : virtual public Config {
 private:
  int address_idx; /** The server's binding IP address */
  int port_idx;    /** The port the server will listen/bind on */

 public:
  SocketConfig();
  ~SocketConfig();

  /**
   * @brief Gets config for socket binding address
   * @returns config string on success, NULL on failure
   */
  uint32_t GetBindingAddress();

  /**
   * @brief Sets config for socket binding address
   * @param addr binding address to set in config
   * @returns 0 on success, -1 on failure
   */
  int SetBindingAddress(uint32_t addr);

  /**
   * @brief Sets config for socket binding address
   * @param addr binding address to set in config
   * @returns 0 on success, -1 on failure
   */
  int SetBindingAddress(const char* addr);

  /**
   * @brief Gets config for socket port
   * @param port port to set in config
   * @returns 0 on success, -1 on failure
   */
  uint16_t GetPort();

  /**
   * @brief Sets config for socket port
   * @param port port to set in config
   * @returns 0 on success, -1 on failure
   */
  int SetPort(uint16_t port);

  /**
   * @brief Loads the socket default configuration
   * @details Clear Key-Val table before using LoadDefaults
   * @returns 0 on success, -1 on failure
   */
  int LoadDefaults();
};