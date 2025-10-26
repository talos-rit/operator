#pragma once

#include "serial/i2c_dev.h"

class DummyI2C : public I2CDev {
 public:
  DummyI2C(int fd, uint8_t addr);
  ~DummyI2C();

  int Echo(uint8_t reg, const uint8_t* msg, uint8_t len);
  int ReadReg(uint8_t reg, uint8_t* msg, uint8_t len);
};