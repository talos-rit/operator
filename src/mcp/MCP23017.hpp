#pragma once

#include <array>
#include <cstdint>
#include <span>
#include <string>

#include "util/file_descriptor.hpp"

class MCP23017 {
 public:
  enum class Register : uint8_t {
    IOCON_A = 0x0A,
    IOCON_B = 0x0B,
    IODIR_A = 0x00,
    IODIR_B = 0x01,
    GPIO_A = 0x12,
    GPIO_B = 0x13,
    GPINTEN_A = 0x04,
    GPINTEN_B = 0x05,
    INTCON_A = 0x08,
    INTCON_B = 0x09,
    DEFVAL_A = 0x06,
    DEFVAL_B = 0x07,
    INTF_A = 0x0E,
    INTF_B = 0x0F,
    INTCAP_A = 0x10,
    INTCAP_B = 0x11,

  };

  enum class Port { A = 0, B = 1 };

  MCP23017(const std::string& device_path, uint8_t address);
  ~MCP23017();

  /* Initializes the MCP23017 by setting all pins as inputs and
   * disabling interrupts.
   */
  bool initialize();

  /* Sets the mirror bit in the IOCON register.
   * When set, INT pins are internally connected.
   */
  bool setMirror();
  /* Sets the mode of a pin on the specified port.
   * port: 0 for GPIOA, 1 for GPIOB
   * isOutput: true for output, false for input
   */
  bool setPinMode(uint8_t pin, Port port, bool isOutput);

  /* Reads the state of a pin on the specified port.
   * port: 0 for GPIOA, 1 for GPIOB
   * Returns true if the pin is HIGH, false if LOW
   */
  bool readPin(uint8_t pin, Port port);

 private:
  uint8_t readRegister(Register reg);
  bool writeRegister(Register reg, uint8_t value);

  FileDescriptor fd_;
  uint8_t address_;
};
