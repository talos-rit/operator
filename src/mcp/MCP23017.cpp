#include "mcp/MCP23017.hpp"

#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>

#include <stdexcept>

MCP23017::MCP23017(const std::string& device_path, uint8_t address)
    : fd_(-1), address_(address) {
  int fd = ::open(device_path.c_str(), O_RDWR);
  if (fd < 0) {
    throw std::runtime_error("Failed to open device: " + device_path);
  }

  if (ioctl(fd, I2C_SLAVE, address_) < 0) {
    ::close(fd);
    throw std::runtime_error("Failed to set I2C address: " +
                             std::to_string(address_));
  }

  fd_ = FileDescriptor(fd);

  prev_pin_states_.fill(false);
  interrupt_modes_.fill(InterruptMode::NONE);

  if (!writeRegister(Register::GPINTEN_A, 0x00)) {
    throw std::runtime_error("Failed to initialize MCP23017");
  }

  if (!writeRegister(Register::GPINTEN_B, 0x00)) {
    throw std::runtime_error("Failed to initialize MCP23017");
  }
}
MCP23017::~MCP23017() {}

bool MCP23017::setMirror() {
  auto iocon = readRegister(Register::IOCON);
  iocon |= (1 << 6);  // Set MIRROR bit while preserving other configuration bits
  return writeRegister(Register::IOCON, iocon);
}

bool MCP23017::setPinMode(uint8_t pin, Port port, bool isOutput) {
  if (pin > 7) {
    throw std::invalid_argument("Invalid pin number");
  }

  Register iodir_reg =
      (port == Port::A) ? Register::IODIR_A : Register::IODIR_B;
  auto iodir = readRegister(iodir_reg);
  if (isOutput) {
    iodir &= ~(1 << pin);  // Set as output
  } else {
    iodir |= (1 << pin);  // Set as input
  }
  return writeRegister(iodir_reg, iodir);
}

bool MCP23017::readPin(uint8_t pin, Port port) {
  if (pin > 7) {
    throw std::invalid_argument("Invalid pin number");
  }

  Register gpio_reg = (port == Port::A) ? Register::GPIO_A : Register::GPIO_B;
  auto gpio = readRegister(gpio_reg);
  return (gpio >> pin) & 0x01;
}

bool MCP23017::setInterrupt(uint8_t pin, Port port, InterruptMode mode) {
  if (pin > 7) {
    throw std::invalid_argument("Invalid pin number");
  }

  Register gpinten_reg =
      (port == Port::A) ? Register::GPINTEN_A : Register::GPINTEN_B;
  auto gpinten = readRegister(gpinten_reg);
  InterruptMode& current_mode =
      interrupt_modes_[static_cast<size_t>(port) * 8 + pin];
  if (mode == InterruptMode::NONE) {
    gpinten &= ~(1 << pin);  // Disable interrupt
  } else {
    gpinten |= (1 << pin);   // Enable interrupt
  }

  current_mode = mode;

  Register intcon_reg =
      (port == Port::A) ? Register::INTCON_A : Register::INTCON_B;
  auto intcon = readRegister(intcon_reg);
  intcon &= ~(1 << pin);  // Default to compare against previous value

  if (!writeRegister(intcon_reg, intcon)) {
    return false;
  }
  // Enable interrupt for the pin
  if (!writeRegister(gpinten_reg, gpinten)) {
    return false;
  }

  return true;
}

std::span<const MCP23017::InterruptPin> MCP23017::getInterruptStatuses(
    MCP23017::Port port) {
  auto intf =
      readRegister((port == Port::A) ? Register::INTF_A : Register::INTF_B);

  size_t count = 0;

  // Fill buffer with active interrupt pins
  for (uint8_t pin = 0; pin < 8; ++pin) {
    auto mode = getInterruptMode(pin, port);
    auto current_state = readPin(pin, port);
    auto prev_state = getPrevPinState(pin, port);

    switch (mode) {
      case InterruptMode::RISING:
        if (current_state && !prev_state) {
          intf |= (1 << pin);
        } else {
          intf &= ~(1 << pin);
        }
        break;
      case InterruptMode::FALLING:
        if (!current_state && prev_state) {
          intf |= (1 << pin);
        } else {
          intf &= ~(1 << pin);
        }
        break;
      case InterruptMode::CHANGE:
        if (current_state != prev_state) {
          intf |= (1 << pin);
        } else {
          intf &= ~(1 << pin);
        }
        break;
      case InterruptMode::NONE:
        intf &= ~(1 << pin);
        break;
    }
    if (intf & (1 << pin)) {
      interrupt_buffer_[count++] = {port, pin};
    }

    // Update previous state
    prev_pin_states_[static_cast<size_t>(port) * 8 + pin] = current_state;
  }

  // Clear interrupt flags by reading GPIO registers
  readRegister(Register::INTCAP_A);
  readRegister(Register::INTCAP_B);

  // Return a span that views ONLY the active portion of the buffer
  return std::span<const InterruptPin>(interrupt_buffer_.data(), count);
}

bool MCP23017::writeRegister(MCP23017::Register reg, uint8_t value) {
  uint8_t buffer[2] = {static_cast<uint8_t>(reg), value};
  ssize_t bytes_written = ::write(fd_.get(), buffer, sizeof(buffer));
  return bytes_written == sizeof(buffer);
}

uint8_t MCP23017::readRegister(MCP23017::Register reg) {
  uint8_t addr = static_cast<uint8_t>(reg);
  ssize_t bytes_written = ::write(fd_.get(), &addr, 1);
  if (bytes_written != 1) {
    throw std::runtime_error("Failed to write register address");
  }

  uint8_t value;
  ssize_t bytes_read = ::read(fd_.get(), &value, 1);
  if (bytes_read != 1) {
    throw std::runtime_error("Failed to read register value");
  }

  return value;
}

MCP23017::InterruptMode MCP23017::getInterruptMode(uint8_t pin, Port port) {
  return interrupt_modes_[static_cast<size_t>(port) * 8 + pin];
}

bool MCP23017::getPrevPinState(uint8_t pin, Port port) {
  return prev_pin_states_[static_cast<size_t>(port) * 8 + pin];
}
