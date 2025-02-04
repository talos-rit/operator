#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <poll.h>
#include <sys/ioctl.h>
#include <linux/gpio.h>
#include <pthread.h>

#include "gpio/test/dummy_gpio.h"

#include "util/comm.h"
#include "util/array.h"
#include "data/list.h"
#include "data/s_list.h"
#include "log/log.h"

#define LOG_CONSOLE_THRESHOLD_THIS  LOG_THRESHOLD_DEFAULT
#define LOG_FILE_THRESHOLD_THIS     LOG_THRESHOLD_MAX

static void* poll_thread(void *arg);

DummyGPIO::DummyGPIO(uint8_t pin)
{
    this->args.pin = pin;

    this->args.chip_fd = open(chip_path, O_RDONLY);
    pthread_create(&pid, NULL, poll_thread, &args);
}

DummyGPIO::~DummyGPIO()
{
    pthread_cancel(pid);
    pthread_join(pid, NULL);

    if (req.fd >= 0) close(req.fd);
    if (args.chip_fd >= 0) close(req.fd);
}

static void* poll_thread(void *arg)
{
    DummyArgs* args = (DummyArgs*) arg;

    struct gpioevent_request event_request;
    memset(&event_request, 0, sizeof(gpioevent_request));
    event_request.lineoffset = args->pin;
    event_request.eventflags = GPIOEVENT_REQUEST_BOTH_EDGES;
    event_request.handleflags = GPIOHANDLE_REQUEST_INPUT | GPIOHANDLE_REQUEST_BIAS_PULL_DOWN;

    int res = ioctl(args->chip_fd, GPIO_GET_LINEEVENT_IOCTL, &event_request);

    if (res < 0) {
       LOG_WARN("Failed requesting events");
       close(args->chip_fd);
       return NULL;
    }

    else LOG_INFO("Established GPIO request on line %d", args->pin);

    #if 0
    struct gpio_v2_line_config config;
    memset(&config, 0, sizeof(struct gpio_v2_line_config));
    config.attrs[0].attr.id = GPIO_V2_LINE_ATTR_ID_DEBOUNCE;
    config.attrs[0].attr.debounce_period_us = 25;
    config.attrs[0].mask = 1;
    config.num_attrs = 1;
    res = ioctl(event_request.fd, GPIO_V2_LINE_SET_CONFIG_IOCTL, &config);

    res = ioctl(event_request.fd, GPIO_V2_LINE_SET_CONFIG_IOCTL, &config);
    if (res < 0) {
       LOG_WARN("Failed reconfiguring debounce.");
       close(args->chip_fd);
       return NULL;
    }
    #endif

   __u64 first_event_timestamp = 0;
   __u64 last_event_timestamp = 0;

    struct gpioevent_data event_data;
    struct pollfd poll_file_descriptor;
    memset(&poll_file_descriptor, 0, sizeof(pollfd));
    poll_file_descriptor.fd = event_request.fd;
    poll_file_descriptor.events = POLLIN;

    while(1)
    {
      int poll_result = poll(&poll_file_descriptor, 1, 1); // time out after 1 milliseconds

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
      LOG_INFO("Event detected");
      if (poll_file_descriptor.revents & POLLIN) {

         int read_result = read(poll_file_descriptor.fd, &event_data, sizeof(event_data));

         if (read_result == -1) {
            // printf("Read error.\n");
            continue;
         }

         if (event_data.id == GPIOEVENT_EVENT_RISING_EDGE) {
            args->count_rising++;
            printf("Rising edge at %llu.\n", event_data.timestamp);
         } else if (event_data.id == GPIOEVENT_EVENT_FALLING_EDGE) {
            args->count_falling++;
            printf("Falling edge at %llu.\n",event_data.timestamp);
         } else {
            // printf("Some other event?\n");
         }

         if (first_event_timestamp == 0) {
            first_event_timestamp = event_data.timestamp;
         } else {
            last_event_timestamp = event_data.timestamp;
         }
      }
    }
}
