#pragma once

#include <linux/gpio.h>
#include <pthread.h>
#include <stdint.h>

typedef struct _dummy_args {
  uint8_t pin;
  int chip_fd;
  __u64 count_rising = 0;
  __u64 count_falling = 0;
} DummyArgs;

class DummyGPIO {
 private:
  DummyArgs args;
  const char* chip_path = "/dev/gpiochip0";
  struct gpio_v2_line_request req;
  pthread_t pid;

 public:
  DummyGPIO(uint8_t pin);
  ~DummyGPIO();

  int8_t read_level();
  int8_t set_level(uint8_t level);
};
