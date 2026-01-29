#include "motorhat/MotorHAT.hpp"

#include "log/log.hpp"

#define LOG_CONSOLE_THRESHOLD_THIS LOG_THRESHOLD_DEFAULT
#define LOG_FILE_THRESHOLD_THIS LOG_THRESHOLD_MAX

MotorHAT::MotorHAT(const std::string& device_path, uint8_t address)
    : pwm_driver_(device_path, address) {
  for (size_t i = 0; i < static_cast<size_t>(Motor::NUM_MOTORS); ++i) {
    auto channels = getMotorChannels(static_cast<Motor>(i));
    // Configure IN1 and IN2 as digital outputs
    pwm_driver_.configureChannel(channels.in1, true);
    pwm_driver_.configureChannel(channels.in2, true);
    // Configure SPEED channel as PWM output
    pwm_driver_.configureChannel(channels.speed, false);
  }
}

MotorHAT::~MotorHAT() {}

bool MotorHAT::initialize() {
  return pwm_driver_.initialize(DEFAULT_FREQUENCY_HZ);
}

bool MotorHAT::setMotorSpeed(Motor motor, uint8_t speed) {
  float duty_cycle = static_cast<float>(speed) / 255.0f;
  auto channels = getMotorChannels(motor);

  LOG_INFO("Setting motor %d speed to %d (duty cycle: %.2f)",
           static_cast<int>(motor), speed, duty_cycle);

  return pwm_driver_.setDutyCycle(channels.speed, duty_cycle);
}

bool MotorHAT::setMotorDirection(Motor motor, Direction direction) {
  auto channels = getMotorChannels(motor);
  switch (direction) {
    case Direction::FORWARD:
      return pwm_driver_.digitalWrite(channels.in1, true) &&
             pwm_driver_.digitalWrite(channels.in2, false);
    case Direction::BACKWARD:
      return pwm_driver_.digitalWrite(channels.in1, false) &&
             pwm_driver_.digitalWrite(channels.in2, true);
    case Direction::RELEASE:
      return pwm_driver_.digitalWrite(channels.in1, false) &&
             pwm_driver_.digitalWrite(channels.in2, false);
    default:
      return false;
  }
}

bool MotorHAT::stopMotor(Motor motor) {
  auto channels = getMotorChannels(motor);
  return pwm_driver_.digitalWrite(channels.in1, false) &&
         pwm_driver_.digitalWrite(channels.in2, false) &&
         pwm_driver_.setDutyCycle(channels.speed, 0.0f);
}

constexpr MotorHAT::MotorChannels MotorHAT::getMotorChannels(Motor motor) {
  auto index = static_cast<size_t>(motor);
  if (index >= 4) index = 0;  // Default to MOTOR1 on invalid input
  return MOTOR_MAP[index];
}
