#pragma once

#include <array>
#include <cstdint>
#include <string>

#include "util/file_descriptor.hpp"

class PCA9685 {
 public:
  constexpr static uint16_t PWM_MAX = 4096;
  enum class Register : uint8_t {
    MODE1 = 0x00,
    MODE2 = 0x01,

    SUBADR1 = 0x02,
    SUBADR2 = 0x03,
    SUBADR3 = 0x04,
    ALLCALLADR = 0x05,

    LED0_ON_L = 0x06,
    LED0_ON_H = 0x07,
    LED0_OFF_L = 0x08,
    LED0_OFF_H = 0x09,

    LED1_ON_L = 0x0A,
    LED1_ON_H = 0x0B,
    LED1_OFF_L = 0x0C,
    LED1_OFF_H = 0x0D,

    LED2_ON_L = 0x0E,
    LED2_ON_H = 0x0F,
    LED2_OFF_L = 0x10,
    LED2_OFF_H = 0x11,

    LED3_ON_L = 0x12,
    LED3_ON_H = 0x13,
    LED3_OFF_L = 0x14,
    LED3_OFF_H = 0x15,

    LED4_ON_L = 0x16,
    LED4_ON_H = 0x17,
    LED4_OFF_L = 0x18,
    LED4_OFF_H = 0x19,

    LED5_ON_L = 0x1A,
    LED5_ON_H = 0x1B,
    LED5_OFF_L = 0x1C,
    LED5_OFF_H = 0x1D,

    LED6_ON_L = 0x1E,
    LED6_ON_H = 0x1F,
    LED6_OFF_L = 0x20,
    LED6_OFF_H = 0x21,

    LED7_ON_L = 0x22,
    LED7_ON_H = 0x23,
    LED7_OFF_L = 0x24,
    LED7_OFF_H = 0x25,

    LED8_ON_L = 0x26,
    LED8_ON_H = 0x27,
    LED8_OFF_L = 0x28,
    LED8_OFF_H = 0x29,

    LED9_ON_L = 0x2A,
    LED9_ON_H = 0x2B,
    LED9_OFF_L = 0x2C,
    LED9_OFF_H = 0x2D,

    LED10_ON_L = 0x2E,
    LED10_ON_H = 0x2F,
    LED10_OFF_L = 0x30,
    LED10_OFF_H = 0x31,

    LED11_ON_L = 0x32,
    LED11_ON_H = 0x33,
    LED11_OFF_L = 0x34,
    LED11_OFF_H = 0x35,

    LED12_ON_L = 0x36,
    LED12_ON_H = 0x37,
    LED12_OFF_L = 0x38,
    LED12_OFF_H = 0x39,

    LED13_ON_L = 0x3A,
    LED13_ON_H = 0x3B,
    LED13_OFF_L = 0x3C,
    LED13_OFF_H = 0x3D,

    LED14_ON_L = 0x3E,
    LED14_ON_H = 0x3F,
    LED14_OFF_L = 0x40,
    LED14_OFF_H = 0x41,

    LED15_ON_L = 0x42,
    LED15_ON_H = 0x43,
    LED15_OFF_L = 0x44,
    LED15_OFF_H = 0x45,

    ALL_LED_ON_L = 0xFA,
    ALL_LED_ON_H = 0xFB,
    ALL_LED_OFF_L = 0xFC,
    ALL_LED_OFF_H = 0xFD,
    PRE_SCALE = 0xFE,
    TESTMODE = 0xFF
  };

  enum class Channel : uint8_t {
    CHANNEL0 = 0,
    CHANNEL1 = 1,
    CHANNEL2 = 2,
    CHANNEL3 = 3,
    CHANNEL4 = 4,
    CHANNEL5 = 5,
    CHANNEL6 = 6,
    CHANNEL7 = 7,
    CHANNEL8 = 8,
    CHANNEL9 = 9,
    CHANNEL10 = 10,
    CHANNEL11 = 11,
    CHANNEL12 = 12,
    CHANNEL13 = 13,
    CHANNEL14 = 14,
    CHANNEL15 = 15,
  };

  struct ChannelRegisters {
    Register on_low;
    Register on_high;
    Register off_low;
    Register off_high;
  };

  PCA9685(const std::string& device_path, uint8_t address);
  ~PCA9685();

  /* Initializes the PCA9685 with the specified PWM frequency in Hz.
   * Returns true on success, false on failure.
   */
  bool initialize(float frequency_hz);

  /* Configures the specified channel for either digital output or PWM output.
   * If digital_mode is true, the channel is set for digital output (on/off).
   * If digital_mode is false, the channel is set for PWM output.
   * Returns true on success, false on failure.
   */
  void configureChannel(Channel channel, bool digital_mode);

  /* Sets the duty cycle for the specified channel.
   * duty_cycle should be in the range [0.0, 1.0], where 0.0 is always off and 1.0 is always on.
   * Returns true on success, false on failure.
   */
  bool setDutyCycle(Channel channel, float duty_cycle);

  /* Sets the digital output for the specified channel.
   * value is true for HIGH and false for LOW.
   * Returns true on success, false on failure.
   */
  bool digitalWrite(Channel channel, bool value);

  /* Retrieves the register addresses for the specified channel.
   */
  static constexpr ChannelRegisters getChannelRegisters(Channel channel);

 private:
  bool readRegister(Register reg, uint8_t& out);
  bool writeRegister(Register reg, uint8_t value);
  bool writeChannelRegisters(Channel channel, uint16_t on, uint16_t off);
  FileDescriptor fd_;
  uint8_t address_;

  // Tracks whether each channel is in digital mode (true) or PWM mode (false)
  std::array<bool, 16> channel_modes_;
};
