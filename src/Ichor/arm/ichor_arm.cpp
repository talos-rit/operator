#include "arm/ichor_arm.hpp"

#include <err.h>
#include <fcntl.h>
#include <stddef.h>
#include <stdio.h>
#include <sys/time.h>
#include <termios.h>
#include <unistd.h>

#include "log/log.hpp"
#include "mcp/MCP23017.hpp"
#include "motorhat/MotorHAT.hpp"
#include "util/comm.hpp"

#define LOG_CONSOLE_THRESHOLD_THIS LOG_THRESHOLD_DEFAULT
#define LOG_FILE_THRESHOLD_THIS LOG_THRESHOLD_MAX

#define ERV_CLOCK CLOCK_REALTIME

Ichor::Ichor(const char* i2c_dev, uint8_t dac0_addr, uint8_t dac1_addr) {
  motor_controllers[0] = new MotorHAT(i2c_dev, dac0_addr);
  mcp_gpio = new MCP23017("/dev/i2c-1", 0x21);
  for (uint8_t pin = 0; pin < 5; pin++) {
    mcp_gpio->setPinMode(pin, MCP23017::Port::A, false);  // Set as input
  }
  // TODO: Add adc
}

Ichor::~Ichor() = default;

bool Ichor::initialize() {
  if (!motor_controllers[0]->initialize()) {
    LOG_ERROR("Failed to initialize MotorHAT controller 0");
    return false;
  }
  return true;
}

void Ichor::poll() {
  // TODO                 // Check ADC values (overcurrent / overexertion)

  usleep(25e3);  // 25 ms delay (defacto delay in Talos Operator so far)
}

int Ichor::handShake() { return 0; }

int Ichor::polarPan(API::PolarPan* pan) {
  switch (oversteer) {
    case OversteerConfig::None:
      break;
    case OversteerConfig::Ignore:
      break;
    case OversteerConfig::Abort:
      break;
    default:
      STD_FAIL;
  }

  uint8_t iter = 0;
  char text[255];

  iter += sprintf(&text[iter], "Polar Pan Payload:\n");
  iter += sprintf(&text[iter], "\tΔ Azimuth: \t%d\n", pan->delta_azimuth);
  iter += sprintf(&text[iter], "\tΔ Altitude: \t%d\n", pan->delta_altitude);
  iter += sprintf(&text[iter], "\tDelay: \t\t%d\n", pan->delay_ms);
  iter += sprintf(&text[iter], "\tTime: \t\t%d\n", pan->time_ms);
  LOG_VERBOSE(4, "%s", text);

  return 0;
}

int Ichor::polarPanStart(API::PolarPanStart* pan) {
  uint8_t iter = 0;
  char text[255];

  if (pan->delta_azimuth != 0) {
    if (!motor_controllers[0]->setMotorDirection(
            MotorHAT::Motor::MOTOR1, (pan->delta_azimuth >= 0)
                                         ? MotorHAT::Direction::FORWARD
                                         : MotorHAT::Direction::BACKWARD)) {
      LOG_ERROR("Failed to set motor direction for azimuth motor");
      return -1;
    }
    if (!motor_controllers[0]->setMotorSpeed(MotorHAT::Motor::MOTOR1, 200)) {
      LOG_ERROR("Failed to set motor speed for azimuth motor");
      return -1;
    }
  }

  if (pan->delta_altitude != 0) {
    if (!motor_controllers[0]->setMotorDirection(
            MotorHAT::Motor::MOTOR3, (pan->delta_altitude >= 0)
                                         ? MotorHAT::Direction::FORWARD
                                         : MotorHAT::Direction::BACKWARD)) {
      LOG_ERROR("Failed to set motor direction for altitude motor");
      return -1;
    }
    if (!motor_controllers[0]->setMotorSpeed(MotorHAT::Motor::MOTOR3, 200)) {
      LOG_ERROR("Failed to set motor speed for altitude motor");
      return -1;
    }
  }

  // static speed of 200 out of 255

  iter += sprintf(&text[iter], "Polar Pan Start Payload:\n");
  iter += sprintf(&text[iter], "\tΔ Azimuth: \t%d\n", pan->delta_azimuth);
  iter += sprintf(&text[iter], "\tΔ Altitude: \t%d\n", pan->delta_altitude);
  LOG_VERBOSE(4, "%s", text);

  return 0;
}

int Ichor::polarPanStop() {
  if (!motor_controllers[0]->stopMotor(MotorHAT::Motor::MOTOR1)) {
    LOG_ERROR("Failed to stop azimuth motor");
    return -1;
  }
  if (!motor_controllers[0]->stopMotor(MotorHAT::Motor::MOTOR3)) {
    LOG_ERROR("Failed to stop altitude motor");
    return -1;
  }
  return 0;
}

int Ichor::home(API::Home* home) {
  uint8_t iter = 0;
  char text[255];

  iter += sprintf(&text[iter], "Home Payload:\n");
  iter += sprintf(&text[iter], "\tDelay: \t\t%d", home->delay_ms);

  return 0;
}
