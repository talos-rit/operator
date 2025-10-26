#include "enc/encoder.h"

RotaryEncoder::RotaryEncoder() {
  value = 0;
  state = 1;
}

RotaryEncoder::~RotaryEncoder() {}

int32_t RotaryEncoder::GetValue() { return value; }

void RotaryEncoder::ToggleA() {
  value += state;
  state *= -1;
}

void RotaryEncoder::ToggleB() {
  value -= state;
  state *= -1;
}