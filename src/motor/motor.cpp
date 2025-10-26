#include "motor/motor.h"

#include <math.h>
#include <stdlib.h>

#include "dac/PCA9685PW.h"
#include "enc/encoder.h"
#include "gpio/isr.h"
#include "log/log.h"
#include "util/comm.h"

#define LOG_FILE_THRESHOLD_THIS LOG_THRESHOLD_MAX
#define LOG_CONSOLE_THRESHOLD_THIS LOG_THRESHOLD_DEFAULT

#define MOTOR_DEAD_SPEED 4095
#define MOTOR_BRAKE_SPEED 0

Motor::Motor(PCA9685PW* dac, uint8_t in1_pin, uint8_t in2_pin,
             uint8_t speed_pin) {
  if (dac) this->dac = dac;  // Optionally linked lated

  // Outputs
  this->speed_pin = speed_pin;
  this->in1_pin = in1_pin;
  this->in2_pin = in2_pin;

  // Inputs
  type = Motor::DriveType::DEAD;

  last_speed = 0;
}

Motor::~Motor() {}

int Motor::RegisterDriver(PCA9685PW* dac) {
  if (!dac) STD_FAIL;
  this->dac = dac;
  return 0;
}

int Motor::Kill() {
  if (!dac) STD_FAIL;
  if (in1_pin >= DAC_PCA_CHANNEL_COUNT) STD_FAIL;
  if (in2_pin >= DAC_PCA_CHANNEL_COUNT) STD_FAIL;
  if (speed_pin >= DAC_PCA_CHANNEL_COUNT) STD_FAIL;

  dac->SetDutyCycle(in1_pin, 0);
  dac->SetDutyCycle(in2_pin, 0);
  dac->SetDutyCycle(speed_pin, DAC_PCA_MAX_DUTY_CYCLE);

  return 0;
}

int Motor::SetSpeed(int16_t speed) {
  uint16_t pwm;
  if (!dac) STD_FAIL;
  if ((pwm = abs(speed)) > DAC_PCA_MAX_DUTY_CYCLE) STD_FAIL;

  if (0 != speed && (0 == last_speed || last_speed * speed < 0)) {
    uint16_t in1 = 0;
    uint16_t in2 = DAC_PCA_MAX_DUTY_CYCLE;

    if (speed < 0) {
      in1 = DAC_PCA_MAX_DUTY_CYCLE;
      in2 = 0;
    }

    dac->SetDutyCycle(in1_pin, in1);
    dac->SetDutyCycle(in2_pin, in2);
  }

  dac->SetDutyCycle(speed_pin, pwm);
  last_speed = speed;
  return 0;
}

int Motor::GetSpeed() { return -1; }