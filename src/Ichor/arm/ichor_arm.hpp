#pragma once

#include "arm/arm.hpp"
#include "mcp/MCP23017.hpp"
#include "motorhat/MotorHAT.hpp"
#include "pwm/PCA9685.hpp"

#define ICHOR_AXIS_COUNT 8

class Ichor : public Arm {
  static constexpr uint8_t ICHOR_HAT_COUNT = 2;

 public:
  Ichor(const char *i2c_dev, uint8_t dac0_addr, uint8_t dac1_addr);
  ~Ichor();
  bool initialize();

 private:
  int i2c_fd;
  MotorHAT *motor_controllers[ICHOR_HAT_COUNT];
  MCP23017 *mcp_gpio;

  char polar_pan_cont;
  bool manual_mode;
  OversteerConfig oversteer;

  int handShake();
  int polarPan(API::PolarPan *pan);
  int polarPanStart(API::PolarPanStart *pan);
  int polarPanStop();
  int home(API::Home *home);
  void poll();
};
