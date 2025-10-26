#include "driver/driver.h"

#include <stdlib.h>

#include "log/log.h"
#include "util/comm.h"

#define LOG_FILE_THRESHOLD_THIS LOG_THRESHOLD_MAX
#define LOG_CONSOLE_THRESHOLD_THIS LOG_THRESHOLD_DEFAULT

Driver::Driver(PCA9685PW* dac, uint8_t in1, uint8_t in2, uint8_t speed,
               IchorISR* isr, uint8_t enc_a, uint8_t enc_b, void* adc,
               uint8_t adc_channel)
    : motor(dac, in1, in2, speed) {
  this->enc_a = enc_a;
  this->enc_b = enc_b;
  isr->RegisterPin(enc_a, GPIO_INTR_TYPE_ENCODER_A, {.enc = &enc});
  isr->RegisterPin(enc_b, GPIO_INTR_TYPE_ENCODER_B, {.enc = &enc});
  this->adc = adc;
  this->adc_channel = adc_channel;

  this->target = 0;
  this->last_velocity = 0;
  this->speed = 0;
  v_func = NULL;
}

Driver::~Driver() {}

int Driver::SetTarget(int32_t target, int32_t start) {
  // TODO: setup bounding area
  this->target = target;
  this->start = start;
  return 0;
}

int Driver::SetSpeedCoefficient(uint8_t speed) {
  if (speed > DRIVER_MAX_SPEED) STD_FAIL;
  this->speed = speed;
  return 0;
}

int Driver::SetVelocityFunc(float (*func)(int32_t target, int32_t start,
                                          int32_t pos)) {
  if (!func) STD_FAIL;
  v_func = func;
  return 0;
}

int Driver::Abort() {
  motor.Kill();
  return 0;
}

int Driver::Shutdown() {
  motor.SetSpeed(0);
  return 0;
}

int Driver::Poll() {
  if (!v_func) STD_FAIL;
  // TODO: Check ADC (overcurrent); if too high, abort
  // Adjust speed according to v_func and encoder/target values
  // ISR.ProcessEvents() should be called just prior to Polling all motors
  int16_t velocity = ((float)(DAC_PCA_MAX_DUTY_CYCLE * speed)) *
                     (v_func(target, start, enc.GetValue()) / 100.0f);
  if (velocity != last_velocity) LOG_INFO("Velocity: %d", velocity);
  // if (velocity * last_velocity < 0) velocity = 0;         // If switching
  // directions, brake
  motor.SetSpeed(velocity);
  last_velocity = velocity;
  return 0;
}