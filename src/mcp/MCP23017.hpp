#pragma once

#include <array>
#include <cstdint>
#include <span>
#include <string>

#include "util/file_descriptor.hpp"

class MCP23017 {
 public:
  enum class Port { A = 0, B = 1 };
  enum class InterruptMode { NONE = 0, RISING = 1, FALLING = 2, CHANGE = 3 };

  struct InterruptPin {
    Port port;
    uint8_t pin;
  };

  MCP23017(const std::string& device_path, uint8_t address);
  ~MCP23017();

  /* Sets the mirror bit in the IOCON register.
   * When set, INT pins are internally connected.
   */

  bool setMirror();
  /* Sets the mode of a pin on the specified port.
   * port: 0 for GPIOA, 1 for GPIOB
   * isOutput: true for output, false for input
   */
  bool setPinMode(uint8_t pin, Port port, bool isOutput);
  bool readPin(uint8_t pin, Port port);
  bool setInterrupt(uint8_t pin, Port port, InterruptMode mode);
  std::span<const InterruptPin> getInterruptStatuses(Port port);

  uint8_t readRegister(uint8_t reg);

 private:
  bool writeRegister(uint8_t reg, uint8_t value);
  InterruptMode getInterruptMode(uint8_t pin, Port port);
  bool getPrevPinState(uint8_t pin, Port port);

  FileDescriptor fd_;
  uint8_t address_;
  std::array<InterruptPin, 16> interrupt_buffer_;
  std::array<InterruptMode, 16> interrupt_modes_;
  std::array<bool, 16> prev_pin_states_;
  static constexpr uint8_t IOCON = 0x0A;
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
  static constexpr uint8_t INTCAP_A = 0x10;
  static constexpr uint8_t INTCAP_B = 0x11;
};
