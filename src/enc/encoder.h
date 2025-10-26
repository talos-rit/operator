#pragma once

#include <stdint.h>

/**
 * @brief Rotary Encoder using stardary (2 bit Gray code) encoding
 */
class RotaryEncoder {
 private:
  int32_t value;
  int8_t state;

 public:
  RotaryEncoder();
  ~RotaryEncoder();

  int32_t GetValue();

  void ToggleA();
  void ToggleB();
};