#pragma once

#include "arm/arm.h"
#include "dac/PCA9685PW.h"
#include "driver/driver.h"
#include "gpio/isr.h"

#define ICHOR_AXIS_COUNT 8

class Ichor : public Arm {
 public:
  Ichor(const char *isr_dev, const char *i2c_dev, uint8_t dac0_addr,
        uint8_t dac1_addr, uint8_t adc_addr);
  ~Ichor();

  int RegisterMotor(uint8_t motor_index, uint8_t dac_index, uint8_t dac_in1,
                    uint8_t dac_in2, uint8_t dac_speed, uint8_t enc_a,
                    uint8_t enc_b, uint8_t adc_channel);

  int Init();

 private:
  int i2c_fd;
  Driver *axis[ICHOR_AXIS_COUNT];
  PCA9685PW *dac[2];
  IchorISR *isr;

  char polar_pan_cont;
  bool manual_mode;
  OversteerConfig oversteer;

  int HandShake();
  int PolarPan(API_Data_Polar_Pan *pan);
  int PolarPanStart(API_Data_Polar_Pan_Start *pan);
  int PolarPanStop();
  int Home(API_Data_Home *home);
  void Poll();
};