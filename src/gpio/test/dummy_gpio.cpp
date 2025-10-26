#include "gpio/test/dummy_gpio.h"

#include <fcntl.h>
#include <linux/gpio.h>
#include <poll.h>
#include <pthread.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "data/list.h"
#include "data/s_list.h"
#include "log/log.h"
#include "util/array.h"
#include "util/comm.h"

#define LOG_CONSOLE_THRESHOLD_THIS LOG_THRESHOLD_DEFAULT
#define LOG_FILE_THRESHOLD_THIS LOG_THRESHOLD_MAX

static void* poll_thread(void* arg);

DummyGPIO::DummyGPIO(uint8_t pin) {
  this->args.pin = pin;

  this->args.chip_fd = open(chip_path, O_RDONLY);
  pthread_create(&pid, NULL, poll_thread, &args);
}

DummyGPIO::~DummyGPIO() {
  pthread_cancel(pid);
  pthread_join(pid, NULL);

  if (req.fd >= 0) close(req.fd);
  if (args.chip_fd >= 0) close(req.fd);
}

static void* poll_thread(void* arg) {
  DummyArgs* args = (DummyArgs*)arg;

  struct gpio_v2_line_request line_request;
  memset(&line_request, 0, sizeof(gpioevent_request));
  line_request.offsets[0] = args->pin;
  line_request.num_lines = 1;
  line_request.config.flags =
      GPIO_V2_LINE_FLAG_INPUT | GPIO_V2_LINE_FLAG_EDGE_RISING |
      GPIO_V2_LINE_FLAG_EDGE_FALLING | GPIO_V2_LINE_FLAG_BIAS_PULL_DOWN |
      GPIO_V2_LINE_FLAG_EVENT_CLOCK_REALTIME | 0;

  gpio_v2_line_config_attribute* debounce = &line_request.config.attrs[0];
  debounce->mask = _BITULL(0);
  debounce->attr.id = GPIO_V2_LINE_ATTR_ID_DEBOUNCE;
  debounce->attr.debounce_period_us = 100;

  line_request.config.num_attrs = 1;

  int res = ioctl(args->chip_fd, GPIO_V2_GET_LINE_IOCTL, &line_request);

  if (res < 0) {
    LOG_WARN("Failed requesting events");
    close(args->chip_fd);
    return NULL;
  }

  else
    LOG_INFO("Established GPIO request on line %d", args->pin);

#if 0
   struct gpio_v2_line_config config;
   memset(&config, 0, sizeof(struct gpio_v2_line_config));
   config.attrs[0].attr.id = GPIO_V2_LINE_ATTR_ID_DEBOUNCE;
   config.attrs[0].attr.debounce_period_us = 25;
   config.attrs[0].mask = 1;
   config.num_attrs = 1;
   res = ioctl(line_request.fd, GPIO_V2_LINE_SET_CONFIG_IOCTL, &config);

   res = ioctl(line_request.fd, GPIO_V2_LINE_SET_CONFIG_IOCTL, &config);
   if (res < 0) {
      LOG_WARN("Failed reconfiguring debounce.");
      close(args->chip_fd);
      return NULL;
   }
#endif

  __u64 first_event_timestamp = 0;
  __u64 last_event_timestamp = 0;

  struct gpio_v2_line_event event_data;
  struct pollfd poll_file_descriptor;
  memset(&poll_file_descriptor, 0, sizeof(pollfd));
  poll_file_descriptor.fd = line_request.fd;
  poll_file_descriptor.events = POLLIN;

  while (1) {
    int poll_result =
        poll(&poll_file_descriptor, 1, 1);  // time out after 1 milliseconds

    if (poll_result == 0) {
      // printf("Poll timeout.\n");
      continue;
    }

    if (poll_result < 0) {
      // printf("Poll error.\n");
      continue;
    }

    if (poll_result > 1) {
      // printf("Multiple events per poll.\n");
    }

    // The "revents" field counts returned events.
    // The "POLLIN" constant seems to be a bitmask.
    LOG_INFO("revents: %04X", poll_file_descriptor.revents);
    if (poll_file_descriptor.revents & POLLIN) {
      int read_result =
          read(poll_file_descriptor.fd, &event_data, sizeof(event_data));

      if (read_result == -1) {
        printf("Read error.\n");
        continue;
      }

      if (event_data.id == GPIO_V2_LINE_EVENT_RISING_EDGE) {
        args->count_rising++;
        printf("Rising edge at %llu.\n", event_data.timestamp_ns);
      } else if (event_data.id == GPIO_V2_LINE_EVENT_FALLING_EDGE) {
        args->count_falling++;
        printf("Falling edge at %llu.\n", event_data.timestamp_ns);
      } else {
        // printf("Some other event?\n");
      }

      if (first_event_timestamp == 0) {
        first_event_timestamp = event_data.timestamp_ns;
      } else {
        last_event_timestamp = event_data.timestamp_ns;
      }
    }
  }
}
