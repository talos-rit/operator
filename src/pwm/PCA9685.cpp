#include "pwm/PCA9685.hpp"

#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>

#include <stdexcept>
#include <thread>

#include "log/log.h"

#define LOG_CONSOLE_THRESHOLD_THIS LOG_THRESHOLD_DEFAULT
#define LOG_FILE_THRESHOLD_THIS LOG_THRESHOLD_MAX

PCA9685::PCA9685(const std::string& device_path, uint8_t address)
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
  // Reset the chip
  if (!writeRegister(Register::MODE1, 0x00)) {
    throw std::runtime_error("Failed to reset PCA9685");
  }
}

PCA9685::~PCA9685() {}

bool PCA9685::initialize(float frequency_hz) {
  if (frequency_hz < 24.0f || frequency_hz > 1526.0f) {
    LOG_ERROR("Frequency %.2f Hz out of range (24-1526 Hz)", frequency_hz);
    return false;
  }

  // Calculate prescale value
  float prescaleval = 25000000.0f;  // 25MHz
  prescaleval /= 4096.0f;           // 12-bit
  prescaleval /= frequency_hz;
  prescaleval -= 1.0f;
  uint8_t prescale = static_cast<uint8_t>(prescaleval + 0.5f);

  // Put the device to sleep to set the prescale
  uint8_t oldmode = readRegister(Register::MODE1);
  uint8_t newmode = (oldmode & 0x7F) | 0x10;  // Sleep
  if (!writeRegister(Register::MODE1, newmode)) {
    LOG_ERROR("Failed to put PCA9685 to sleep");
    return false;
  }

  // Set the prescale
  if (!writeRegister(Register::PRE_SCALE, prescale)) {
    LOG_ERROR("Failed to set PCA9685 prescale");
    return false;
  }

  // Wake up the device
  if (!writeRegister(Register::MODE1, oldmode)) {
    LOG_ERROR("Failed to wake PCA9685 from sleep");
    return false;
  }

  // Wait for oscillator to stabilize
  std::this_thread::sleep_for(std::chrono::milliseconds(5));

  // Enable auto-increment
  if (!writeRegister(Register::MODE1, oldmode | 0xA0)) {
    LOG_ERROR("Failed to enable auto-increment on PCA9685");
    return false;
  }

  return true;
}

void PCA9685::configureChannel(Channel channel, bool digital_mode) {
  if (digital_mode) {
    // Set FULL_OFF bit to disable PWM for this channel
    channel_modes_[static_cast<size_t>(channel)] = true;
  } else {
    // Clear FULL_OFF bit to enable PWM for this channel
    channel_modes_[static_cast<size_t>(channel)] = false;
  }
}

bool PCA9685::setDutyCycle(Channel channel, float duty_cycle) {
  if (duty_cycle < 0.0f || duty_cycle > 1.0f) {
    return false;
  }

  if (channel_modes_[static_cast<size_t>(channel)]) {
    // Channel is in digital mode; cannot set duty cycle
    return false;
  }

  if (duty_cycle >= 1.0f) {
    // Special case: 100% duty cycle
    return writeChannelRegisters(channel, 0x1000, 0);
  } else if (duty_cycle <= 0.0f) {
    // Special case: 0% duty cycle
    return writeChannelRegisters(channel, 0, 0x1000);
  }

  uint16_t off_value = static_cast<uint16_t>(duty_cycle * PWM_MAX);
  return writeChannelRegisters(channel, 0, off_value);
}

bool PCA9685::digitalWrite(Channel channel, bool value) {
  if (channel_modes_[static_cast<size_t>(channel)] == false) {
    // Channel is in PWM mode; cannot perform digital write
    return false;
  }
  ChannelRegisters regs = GetChannelRegisters(channel);
  if (value) {
    bool remove_full_off =
        writeRegister(regs.off_high, 0x00);  // Clear FULL_OFF bit
    bool set_full_on = writeRegister(
        regs.on_high, static_cast<uint8_t>(1 << 4));  // Set FULL_ON bit
    return remove_full_off && set_full_on;

  } else {
    bool remove_full_on =
        writeRegister(regs.on_high, 0x00);  // Clear FULL_ON bit
    bool set_full_off = writeRegister(
        regs.off_high, static_cast<uint8_t>(1 << 4));  // Set FULL_OFF bit
    return remove_full_on && set_full_off;
  }
}

bool PCA9685::writeChannelRegisters(Channel channel, uint16_t on,
                                    uint16_t off) {
  if (on > PWM_MAX || off > PWM_MAX) {
    return false;
  }
  ChannelRegisters regs = GetChannelRegisters(channel);
  uint8_t buffer[5];
  buffer[0] = static_cast<uint8_t>(regs.on_low);

  // The PCA9685 uses 12-bit PWM values for each channel (0–4095).
  // These 12 bits are split across two 8-bit registers:
  //   LEDn_ON_L  -> bits  7..0  (low byte)
  //   LEDn_ON_H  -> bits 11..8  (lower 4 bits of the high byte; upper 4 bits
  //   are control flags)
  // Same layout applies for OFF_L / OFF_H.
  //
  // Therefore:
  //   - (value & 0xFF) extracts the lower 8 data bits for the *_L register.
  //   - ((value >> 8) & 0x0F) extracts only the upper 4 PWM bits for the *_H
  //   register,
  //     while ensuring the control bits (FULL_ON/FULL_OFF in bits 4–7) are not
  //     overwritten.
  //
  // 0xFF = 1111 1111b  -> mask for low byte
  // 0x0F = 0000 1111b  -> mask for PWM bits 11..8 in high byte
  //
  buffer[1] = static_cast<uint8_t>(on & 0xFF);
  buffer[2] = static_cast<uint8_t>((on >> 8) & 0x1F);
  buffer[3] = static_cast<uint8_t>(off & 0xFF);
  buffer[4] = static_cast<uint8_t>((off >> 8) & 0x1F);

  LOG_INFO("Writing to channel %d: ON=%d OFF=%d", static_cast<int>(channel), on,
           off);
  LOG_INFO("Buffer: [%02X %02X %02X %02X %02X]", buffer[0], buffer[1],
           buffer[2], buffer[3], buffer[4]);

  ssize_t bytes_written = ::write(fd_.get(), buffer, sizeof(buffer));
  return bytes_written == sizeof(buffer);
};

uint8_t PCA9685::readRegister(Register reg) {
  uint8_t reg_addr = static_cast<uint8_t>(reg);
  ssize_t bytes_written = ::write(fd_.get(), &reg_addr, sizeof(reg_addr));
  if (bytes_written != sizeof(reg_addr)) {
    return 0;  // Error
  }

  uint8_t value = 0;
  ssize_t bytes_read = ::read(fd_.get(), &value, sizeof(value));
  if (bytes_read != sizeof(value)) {
    return 0;  // Error
  }

  return value;
}

bool PCA9685::writeRegister(Register reg, uint8_t value) {
  uint8_t buffer[2] = {static_cast<uint8_t>(reg), value};
  ssize_t bytes_written = ::write(fd_.get(), buffer, sizeof(buffer));
  return bytes_written == sizeof(buffer);
}

constexpr PCA9685::ChannelRegisters PCA9685::GetChannelRegisters(
    PCA9685::Channel ch) {
  const uint8_t base_addr = static_cast<uint8_t>(0x06 + 4 * static_cast<uint8_t>(ch));
  return {
      static_cast<Register>(base_addr + 0),  // ON_L
      static_cast<Register>(base_addr + 1),  // ON_H
      static_cast<Register>(base_addr + 2),  // OFF_L
      static_cast<Register>(base_addr + 3)   // OFF_H
  };
}
