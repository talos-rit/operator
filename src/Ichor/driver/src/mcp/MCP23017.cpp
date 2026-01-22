#include "mcp/MCP23017.hpp"

#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>

#include <cstdint>
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

bool MCP23017::initialize() {
  if (!writeRegister(Register::GPINTEN_A, 0x00)) {
    return false;
  }

  if (!writeRegister(Register::GPINTEN_B, 0x00)) {
    return false;
  }

  return true;
}

bool MCP23017::setMirror() {
  bool ret;
  uint8_t iocon_a, iocon_b;
  ret = readRegister(Register::IOCON_A, iocon_a);
  if (!ret) {
    return false;
  }
  ret = readRegister(Register::IOCON_B, iocon_b);
  if (!ret) {
    return false;
  }
  iocon_a |=
      (1 << 6);  // Set MIRROR bit while preserving other configuration bits
  iocon_b |=
      (1 << 6);  // Set MIRROR bit while preserving other configuration bits
  bool success_a = writeRegister(Register::IOCON_A, iocon_a);
  bool success_b = writeRegister(Register::IOCON_B, iocon_b);
  return success_a && success_b;
}

bool MCP23017::setPinMode(uint8_t pin, Port port, bool isOutput) {
  if (pin > 7) {
    return false;
  }

  Register iodir_reg =
      (port == Port::A) ? Register::IODIR_A : Register::IODIR_B;
  uint8_t iodir;
  bool ret = readRegister(iodir_reg, iodir);
  if (!ret) {
    return false;
  }
  if (isOutput) {
    iodir &= ~(1 << pin);  // Set as output
  } else {
    iodir |= (1 << pin);  // Set as input
  }
  return writeRegister(iodir_reg, iodir);
}

bool MCP23017::readPin(uint8_t pin, Port port, bool& out) {
  if (pin > 7) {
    return false;
  }

  Register gpio_reg = (port == Port::A) ? Register::GPIO_A : Register::GPIO_B;
  uint8_t gpio;
  bool ret = readRegister(gpio_reg, gpio);
  if (!ret) {
    return false;
  }
  out = (gpio >> pin) & 0x01;
  return true;
}


bool MCP23017::writeRegister(MCP23017::Register reg, uint8_t value) {
  uint8_t buffer[2] = {static_cast<uint8_t>(reg), value};
  ssize_t bytes_written = ::write(fd_.get(), buffer, sizeof(buffer));
  return bytes_written == sizeof(buffer);
}

bool MCP23017::readRegister(MCP23017::Register reg, uint8_t& out) {
  uint8_t addr = static_cast<uint8_t>(reg);
  ssize_t bytes_written = ::write(fd_.get(), &addr, 1);
  if (bytes_written != 1) {
    return false;
  }

  uint8_t value;
  ssize_t bytes_read = ::read(fd_.get(), &value, 1);
  if (bytes_read != 1) {
    return false;
  }

  out = value;
  return true;
}
