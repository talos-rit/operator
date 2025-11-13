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
}
MCP23017::~MCP23017() {}

bool MCP23017::setMirror() { return writeRegister(IOCON, 1 << 6); }

bool MCP23017::setPinMode(uint8_t pin, Port port, bool isOutput) {
  if (pin > 7) {
    throw std::invalid_argument("Invalid pin number");
  }

  uint8_t iodir_reg = (port == Port::A) ? IODIR_A : IODIR_B;
  uint8_t iodir = readRegister(iodir_reg);
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

  uint8_t gpio_reg = (port == Port::A) ? GPIO_A : GPIO_B;
  uint8_t gpio = readRegister(gpio_reg);
  return (gpio >> pin) & 0x01;
}

bool MCP23017::setInterrupt(uint8_t pin, Port port, InterruptMode mode) {
  if (pin > 7) {
    throw std::invalid_argument("Invalid pin number");
  }

  uint8_t gpinten_reg = (port == Port::A) ? GPINTEN_A : GPINTEN_B;
  uint8_t gpinten = readRegister(gpinten_reg);

  uint8_t intcon_reg = (port == Port::A) ? INTCON_A : INTCON_B;
  uint8_t intcon = readRegister(intcon_reg);

  uint8_t defval_reg = (port == Port::A) ? DEFVAL_A : DEFVAL_B;
  uint8_t defval = readRegister(defval_reg);

  switch (mode) {
    case InterruptMode::NONE:
      // Disable interrupt for the pin
      gpinten &= ~(1 << pin);
      intcon &= ~(1 << pin);
      break;
    case InterruptMode::RISING:
      // Interrupt on rising edge
      intcon |= (1 << pin);
      defval &= ~(1 << pin);
      break;
    case InterruptMode::FALLING:
      // Interrupt on falling edge
      intcon |= (1 << pin);
      defval |= (1 << pin);
      break;
    case InterruptMode::CHANGE:
      // Interrupt on any change
      intcon &= ~(1 << pin);
      break;
    default:
      throw std::invalid_argument("Invalid interrupt mode");
  }

  if (!writeRegister(intcon_reg, intcon)) {
    return false;
  }
  if (!writeRegister(defval_reg, defval)) {
    return false;
  }

  // Enable interrupt for the pin
  gpinten |= (1 << pin);
  if (!writeRegister(gpinten_reg, gpinten)) {
    return false;
  }

  return true;
}

std::span<const MCP23017::InterruptPin> MCP23017::getInterruptStatuses() {
  uint8_t intf_a = readRegister(INTF_A);
  uint8_t intf_b = readRegister(INTF_B);

  size_t count = 0;

  // Fill buffer with active interrupt pins
  for (uint8_t pin = 0; pin < 8; ++pin) {
    if (intf_a & (1 << pin)) {
      interrupt_buffer_[count++] = {Port::A, pin};
    }
    if (intf_b & (1 << pin)) {
      interrupt_buffer_[count++] = {Port::B, pin};
    }
  }

  // Clear interrupt flags by reading GPIO registers
  readRegister(INTCAP_A);
  readRegister(INTCAP_B);

  // Return a span that views ONLY the active portion of the buffer
  return std::span<const InterruptPin>(interrupt_buffer_.data(), count);
}

bool MCP23017::writeRegister(uint8_t reg, uint8_t value) {
  uint8_t buffer[2] = {reg, value};
  ssize_t bytes_written = write(fd_.get(), buffer, sizeof(buffer));
  return bytes_written == sizeof(buffer);
}

uint8_t MCP23017::readRegister(uint8_t reg) {
  ssize_t bytes_written = write(fd_.get(), &reg, 1);
  if (bytes_written != 1) {
    throw std::runtime_error("Failed to write register address");
  }

  uint8_t value;
  ssize_t bytes_read = read(fd_.get(), &value, 1);
  if (bytes_read != 1) {
    throw std::runtime_error("Failed to read register value");
  }

  return value;
}
