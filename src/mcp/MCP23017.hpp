#pragma once

#include <cstdint>
#include <string>

#include "util/file_descriptor.hpp"

enum class Port { A = 0, B = 1 };

class MCP23017 {
 public:
  MCP23017(std::string device_path, uint8_t address);
  ~MCP23017();

  /* Sets the mode of a pin on the specified port.
   * port: 0 for GPIOA, 1 for GPIOB
   * isOutput: true for output, false for input
   */
  bool setPinMode(uint8_t pin, Port port, bool isOutput);
  bool readPin(uint8_t pin, Port port);
  bool writePin(uint8_t pin, Port port, bool value);

 private:
  bool writeRegister(uint8_t reg, uint8_t value);
  uint8_t readRegister(uint8_t reg);

  FileDescriptor fd_;
  uint8_t address_;
  static constexpr uint8_t IODIR_A = 0x00;
  static constexpr uint8_t IODIR_B = 0x01;
  static constexpr uint8_t GPIO_A = 0x12;
  static constexpr uint8_t GPIO_B = 0x13;
};
