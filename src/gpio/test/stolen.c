// GPIO sample program: sends data on GPIO 17, receives on GPIO 27.
// Public domain.

#include <errno.h>
#include <fcntl.h>       // open(), O_RDONLY
#include <linux/gpio.h>  // everything about GPIOs
#include <poll.h>        // poll()
#include <pthread.h>     // threading
#include <stdio.h>       // printf()
#include <string.h>
#include <sys/ioctl.h>  // ioctl()
#include <unistd.h>     // close()

// Chip 0 on older Pi models, chip 4 on Pi 5.
#define CHIP "/dev/gpiochip0"

// GPIO line numbers.
#define WRITE_GPIO 17
#define READ_GPIO 4

// Termination flag.
// This is not an example of good coding style.

volatile int TERM = 0;

// Data structures from Linux GPIO libs.
// These here are global because a thread needs them.
// This is not an example of good coding style.

struct gpio_v2_line_request event_request;

struct pollfd poll_file_descriptor;

void *reader(void *arg) {
  // Elevate reader thread priority and use a FIFO scheduler.

  pthread_t thread = pthread_self();
  // const struct sched_param sparam = { .sched_priority = 99, };
  // int res = pthread_setschedparam(thread, SCHED_FIFO, &sparam);
  // if (res != 0) printf("Failed to elevate reader thread priority!\n");

  struct gpio_v2_line_event event_data[32];

  poll_file_descriptor.fd = event_request.fd;
  poll_file_descriptor.events = POLLIN;

  // Some variables for evaluating throughput

  __u64 first_event_timestamp = 0;
  __u64 last_event_timestamp = 0;
  __u64 count_rising = 0;
  __u64 count_falling = 0;

  // Receive loop

  while (TERM == 0) {
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

    if (poll_file_descriptor.revents & POLLIN) {
      while (1) {
        int read_result = read(event_request.fd, &event_data,
                               sizeof(struct gpio_v2_line_event));

        if (read_result == -1) {
          printf("Error: (%d) %s\n", errno, strerror(errno));
          continue;
        }

        else if (read_result == 0) {
          // Done
          printf("Done.\n");
          break;
        }

        else {
          printf("MORE: %d\n", read_result);
        }

        if (event_data[0].id == GPIO_V2_LINE_EVENT_RISING_EDGE) {
          count_rising++;
          // printf("Rising edge at %llu.\n", event_data.timestamp);
        } else if (event_data[0].id == GPIO_V2_LINE_EVENT_FALLING_EDGE) {
          count_falling++;
          // printf("Falling edge at %llu.\n",event_data.timestamp);
        } else {
          // printf("Some other event?\n");
        }

        if (first_event_timestamp == 0) {
          first_event_timestamp = event_data[0].timestamp_ns;
        } else {
          last_event_timestamp = event_data[0].timestamp_ns;
        }
      }
    }
  }

  printf("Received %llu rising and %llu falling edges.\n", count_rising,
         count_falling);

  __u64 duration = last_event_timestamp - first_event_timestamp;
  double seconds = ((double)duration / (double)1000000000);

  printf("Total duration %llu ns (%f s).\n", duration, seconds);

  __u64 nanos_per_edge = duration / (count_rising + count_falling);

  printf("Average %llu ns (%llu microseconds) per edge.\n", nanos_per_edge,
         (nanos_per_edge / 1000));

  __u64 per_second = count_rising / seconds;

  printf("Rising edge frequency %llu Hz.\n", per_second);

  close(poll_file_descriptor.fd);

  return 0;
}

int main(int argc, char *const *argv) {
  int res;  // various call results

  // Elevate main thread priority and use a FIFO scheduler.

  // const struct sched_param sparam = { .sched_priority = 99, };
  // res = sched_setscheduler(0, SCHED_FIFO, &sparam);
  // if (res != 0) printf("Failed to elevate main thread priority!\n");

  int file_descriptor = open(CHIP, O_RDONLY);

  if (file_descriptor < 0) {
    printf("Failed opening GPIO chip.\n");
    return 1;
  }

  // Request events on the reading line.
  memset(&event_request, 0, sizeof(struct gpio_v2_line_request));
  event_request.offsets[0] = READ_GPIO;
  event_request.num_lines = 1;
  event_request.config.flags =
      GPIO_V2_LINE_FLAG_EDGE_RISING | GPIO_V2_LINE_FLAG_EDGE_FALLING |
      GPIO_V2_LINE_FLAG_BIAS_PULL_DOWN | GPIO_V2_LINE_FLAG_INPUT;
  event_request.event_buffer_size = 16;

  res = ioctl(file_descriptor, GPIO_V2_GET_LINE_IOCTL, &event_request);

  if (res < 0) {
    printf("Failed requesting events.\n");
    close(file_descriptor);
    return 1;
  }

  // Request handle on writing line. Many can be requested instead of one.
  struct gpio_v2_line_request handle_request;
  memset(&handle_request, 0, sizeof(struct gpio_v2_line_request));

  handle_request.offsets[0] = WRITE_GPIO;
  handle_request.num_lines = 1;
  event_request.config.flags = GPIO_V2_LINE_FLAG_OUTPUT;

  res = ioctl(file_descriptor, GPIO_V2_GET_LINE_IOCTL, &handle_request);

  if (res < 0) {
    printf("Failed requesting write handle.\n");
    close(file_descriptor);
    return 1;
  }

  // Start a reader thread
  pthread_t reader_thread;
  pthread_create(&reader_thread, NULL, &reader, NULL);

  // Data handle for writing
  struct gpio_v2_line_values hdata;
  memset(&hdata, 0, sizeof(struct gpio_v2_line_values));

  // Time variables for sleeping a short interval.
  struct timespec ts, tr;
  ts.tv_sec = 0;

  printf("------------ now writing and reading -------------------\n");

  hdata.mask = _BITULL(0);
// Write something out ad hope the reader reads it
#if 0
   for (int i = 0; i < 1000; i++) {

      hdata.bits |= _BITULL(0);
      res = ioctl(handle_request.fd, GPIO_V2_LINE_SET_VALUES_IOCTL, &hdata);
      if (res == -1) printf("Failed setting line value.\n");

      // Try to sleep for 5000 nanoseconds.
      // In reality, due to call latencies, you end up sleeping longer.

      ts.tv_nsec = 5000;
      nanosleep(&ts, &tr);

      hdata.bits &= ~_BITULL(0);
      res = ioctl(handle_request.fd, GPIO_V2_LINE_SET_VALUES_IOCTL, &hdata);
      if (res == -1) printf("Failed setting line value.\n");

      ts.tv_nsec = 5000;
      nanosleep(&ts, &tr);
   }
#else
  while (1);
#endif

  close(handle_request.fd);

  // Tell the thread to finish
  TERM = 1;

  // Give it time
  sleep(1);

  // Close resources
  close(file_descriptor);

  // Rejoin main thread with finished thread
  pthread_join(reader_thread, NULL);

  return 0;
}