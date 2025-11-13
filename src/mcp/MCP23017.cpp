#include "MCP23017.hpp"

#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>

#include <stdexcept>

MCP23017::MCP23017(std::string device_path, uint8_t address)
    : fd_(-1), address_(address) {
  int fd = ::open(device_path.c_str(), O_RDWR);
  if (fd < 0) {
    throw std::runtime_error("Failed to open device: " + device_path);
  }

  if (ioctl(fd_, I2C_SLAVE, address_) < 0) {
    ::close(fd);
    throw std::runtime_error("Failed to set I2C address: " +
                             std::to_string(address_));
  }

  fd_ = FileDescriptor(fd);
}
MCP23017::~MCP23017() {}

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
