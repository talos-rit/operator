#pragma once

#define ISR_PIN_MAP_LEN 40

#include <linux/gpio.h>

#include "enc/encoder.h"
#include "gpio/pin.h"

#define ISR_INPUT_DEBOUNCE_US \
  100  // TODO: figure out how to implement this (prevents vast majority of
       // events from registering when active)
#define ISR_CHIP_PATH "/dev/gpiochip0"

class IchorISR {
 private:
  int chip_fd;              /** File descriptor for GPIO chip */
  gpio_v2_line_request req; /** Request struct for interrupt queue */
  GPIO_Interrupt pin_map[ISR_PIN_MAP_LEN]; /** GPIO Offset-to-target map */

  int openISR();
  int closeISR();
  int ExecutePin(uint8_t pin);

 public:
  IchorISR(const char* chip_path);
  ~IchorISR();

  int RegisterPin(uint8_t pin, GPIO_INTR_TYPE type, GPIO_INTR_TARGET target);
  int UnregisterPin(uint8_t pin);
  int AllocatePins();
  void ProcessEvents();

  // TODO: determine how many threads will manage Ichor (will this have its own
  // thread?)
};