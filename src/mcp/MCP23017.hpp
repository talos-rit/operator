#pragma once

#include <array>
#include <cstdint>
#include <span>
#include <string>

#include "util/file_descriptor.hpp"

class MCP23017 {
  enum class Port { A = 0, B = 1 };
  enum class InterruptMode { NONE = 0, RISING = 1, FALLING = 2, CHANGE = 3 };

  struct InterruptPin {
    Port port;
    uint8_t pin;
  };

 public:
  MCP23017(std::string device_path, uint8_t address);
  ~MCP23017();

  /* Sets the mode of a pin on the specified port.
   * port: 0 for GPIOA, 1 for GPIOB
   * isOutput: true for output, false for input
   */
  bool setPinMode(uint8_t pin, Port port, bool isOutput);
  bool readPin(uint8_t pin, Port port);
  bool setInterrupt(uint8_t pin, Port port, InterruptMode mode);
  std::span<const InterruptPin> getInterruptStatuses();

 private:
  bool writeRegister(uint8_t reg, uint8_t value);
  uint8_t readRegister(uint8_t reg);

  FileDescriptor fd_;
  uint8_t address_;
  std::array<InterruptPin, 16> interrupt_buffer_;
  static constexpr uint8_t IODIR_A = 0x00;
  static constexpr uint8_t IODIR_B = 0x01;
  static constexpr uint8_t GPIO_A = 0x12;
  static constexpr uint8_t GPIO_B = 0x13;
  static constexpr uint8_t GPINTEN_A = 0x04;
  static constexpr uint8_t GPINTEN_B = 0x05;
  static constexpr uint8_t INTCON_A = 0x08;
  static constexpr uint8_t INTCON_B = 0x09;
  static constexpr uint8_t DEFVAL_A = 0x06;
  static constexpr uint8_t DEFVAL_B = 0x07;
  static constexpr uint8_t INTF_A = 0x0E;
  static constexpr uint8_t INTF_B = 0x0F;
};
