#pragma once

#include <cstdint>
#include <string>

#include "pwm/PCA9685.hpp"
class MotorHAT {
 public:
  static constexpr uint8_t MAX_SPEED = 255;
  static constexpr float DEFAULT_FREQUENCY_HZ = 1526.0f;
  enum class Motor { MOTOR1, MOTOR2, MOTOR3, MOTOR4, NUM_MOTORS };
  enum class Direction { FORWARD, BACKWARD, RELEASE };

  struct MotorChannels {
    PCA9685::Channel in1;
    PCA9685::Channel in2;
    PCA9685::Channel speed;
  };

  static constexpr MotorChannels MOTOR_MAP[] = {
      {PCA9685::Channel::CHANNEL10, PCA9685::Channel::CHANNEL9,
       PCA9685::Channel::CHANNEL8},
      {PCA9685::Channel::CHANNEL11, PCA9685::Channel::CHANNEL12,
       PCA9685::Channel::CHANNEL13},
      {PCA9685::Channel::CHANNEL4, PCA9685::Channel::CHANNEL3,
       PCA9685::Channel::CHANNEL2},
      {PCA9685::Channel::CHANNEL5, PCA9685::Channel::CHANNEL6,
       PCA9685::Channel::CHANNEL7}};

  MotorHAT(const std::string& device_path, uint8_t address);
  ~MotorHAT();

  /* Initialize the Motor HAT by setting up the PCA9685 driver */
  bool initialize();

  /* Set the speed of a motor (0 to MAX_SPEED) */
  bool setMotorSpeed(Motor motor, uint8_t speed);

  /* Set the direction of a motor (FORWARD, BACKWARD, RELEASE) */
  bool setMotorDirection(Motor motor, Direction direction);

  /* Stop a motor */
  bool stopMotor(Motor motor);

  static constexpr MotorChannels GetMotorChannels(Motor motor);

 private:
  PCA9685 pwm_driver_;
};
