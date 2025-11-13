#include "arm/ichor_arm.h"

#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <termios.h>
#include <unistd.h>

#include "log/log.h"
#include "mcp/MCP23017.hpp"
#include "motor/motor.h"

#define LOG_CONSOLE_THRESHOLD_THIS LOG_THRESHOLD_DEFAULT
#define LOG_FILE_THRESHOLD_THIS LOG_THRESHOLD_MAX

#define ERV_CLOCK CLOCK_REALTIME

static float dead_vel(int32_t target, int32_t start, int32_t pos) { return 0; }

static float open_loop_vel(int32_t target, int32_t start, int32_t pos) {
  if (target == 0) return 0;
  return target > 0 ? 1 : -1;
}

static void set_velocity(Driver **axis,
                         float (*vel)(int32_t target, int32_t start,
                                      int32_t pos)) {
  for (uint8_t idx = 0; idx < ICHOR_AXIS_COUNT; idx++) {
    if (axis[idx]) axis[idx]->SetVelocityFunc(vel);
  }
}

Ichor::Ichor(const char *isr_dev, const char *i2c_dev, uint8_t dac0_addr,
             uint8_t dac1_addr, uint8_t adc_addr) {
  int i2c_fd = open(i2c_dev, O_RDWR);
  if (i2c_fd < 0)
    LOG_WARN("Failed to open I2C bus");
  else
    LOG_INFO("Successfully opened I2C bus");

  isr = new IchorISR(isr_dev);

  dac[0] = new PCA9685PW(i2c_fd, dac0_addr);
  // dac[1] = new PCA9685PW(i2c_fd, dac1_addr);
  mcp_gpio = new MCP23017("/dev/i2c-1", 0x21);
  for (uint8_t pin = 0; pin < 5; pin++) {
    mcp_gpio->setPinMode(pin, MCP23017::Port::A, false);  // Set as input
    // mcp_gpio->setInterrupt(pin, MCP23017::Port::A,
    //                        MCP23017::InterruptMode::FALLING);
  }
  // TODO: Add adc

  for (uint8_t idx = 0; idx < ICHOR_AXIS_COUNT; idx++) axis[idx] = NULL;
}

Ichor::~Ichor() {
  // dac[0].ResetDevice(); // Resets all PCA9685PW devices on bus
}

int Ichor::RegisterMotor(uint8_t motor_index, uint8_t dac_index,
                         uint8_t dac_in1, uint8_t dac_in2, uint8_t dac_speed,
                         uint8_t enc_a, uint8_t enc_b, uint8_t adc_channel) {
  if (motor_index > 8 || dac_index > 1) STD_FAIL;
  axis[motor_index] = new Driver(dac[dac_index], dac_in1, dac_in2, dac_speed,
                                 isr, enc_a, enc_b, NULL, adc_channel);
  axis[motor_index]->SetSpeedCoefficient(50);
  axis[motor_index]->SetVelocityFunc(&dead_vel);
  return 0;
}

int Ichor::Init() {
  isr->AllocatePins();
  return 0;
}

void Ichor::poll() {
  isr->ProcessEvents();  // Check GPIO interrupts (could be an abort signal)
  // TODO                 // Check ADC values (overcurrent / overexertion)
  for (uint8_t idx = 0; idx < ICHOR_AXIS_COUNT; idx++) {
    if (!axis[idx]) continue;
    axis[idx]->Poll();  // Update motors with new control information
  }

  uint8_t intfa = mcp_gpio->readRegister(0x0E);
  uint8_t intfb = mcp_gpio->readRegister(0x0F);

  if (intfa || intfb) {
    LOG_WARN("GPIO Interrupt detected: INTF_A=0x%02X, INTF_B=0x%02X", intfa,
             intfb);
    // Clear interrupt flags by reading INTCAP registers
    mcp_gpio->readRegister(0x10);
    mcp_gpio->readRegister(0x11);
  }


  dac[0]->FlushQueues();  // Flush pending DAC writes
  usleep(25e3);  // 25 ms delay (defacto delay in Talos Operator so far)
}

int Ichor::handShake() { return 0; }

int Ichor::polarPan(API::PolarPan *pan) {
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

int Ichor::polarPanStart(API::PolarPanStart *pan) {
  uint8_t iter = 0;
  char text[255];

  if (!axis[0] || !axis[2]) STD_FAIL;

  axis[0]->SetVelocityFunc(&open_loop_vel);
  axis[0]->SetTarget(pan->delta_azimuth, 0);

  axis[2]->SetVelocityFunc(&open_loop_vel);
  axis[2]->SetTarget(pan->delta_altitude, 0);

  iter += sprintf(&text[iter], "Polar Pan Start Payload:\n");
  iter += sprintf(&text[iter], "\tΔ Azimuth: \t%d\n", pan->delta_azimuth);
  iter += sprintf(&text[iter], "\tΔ Altitude: \t%d\n", pan->delta_altitude);
  LOG_VERBOSE(4, "%s", text);

  return 0;
}

int Ichor::polarPanStop() {
  axis[0]->SetVelocityFunc(&dead_vel);
  axis[2]->SetVelocityFunc(&dead_vel);
  return 0;
}

int Ichor::home(API::Home *home) {
  uint8_t iter = 0;
  char text[255];

  iter += sprintf(&text[iter], "Home Payload:\n");
  iter += sprintf(&text[iter], "\tDelay: \t\t%d", home->delay_ms);

  return 0;
}
