#include "erv_arm/erv.h"

#include <err.h>
#include "util/comm.h"
#include <fcntl.h>
#include <signal.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <termios.h>
#include <unistd.h>

#include "acl/acl.h"
#include "log/log.h"

#define LOG_CONSOLE_THRESHOLD_THIS LOG_THRESHOLD_DEFAULT
#define LOG_FILE_THRESHOLD_THIS LOG_THRESHOLD_MAX

#define ERV_DEFAULT_COMMAND_DELAY 200000
#define ERV_TTY_BUFFER_LEN 127

#define ERV_CLOCK CLOCK_REALTIME

Scorbot::Scorbot(const char *dev) {
  // Setup device
  LOG_VERBOSE(4, "Scorbot device path: %s", dev);
  strcpy(&this->dev[0], &dev[0]);
  fd = open(dev, O_RDWR | O_NOCTTY | O_NDELAY);
  if (fd < 0) {
    LOG_ERROR("Could not open Scorbot device path: %s", strerror(errno));
    raise(SIGABRT);
    return;
  }

  // Setup config
  polar_pan_cont = '\0';
  manual_mode = false;
  oversteer = OVERSTEER_ABORT;

  // Setup ACL
  ACL_init();
  DATA_S_List_init(&cmd_buffer);
  gettimeofday(&last_start, NULL);

  ACL_flush_tx(&cmd_buffer);
}

Scorbot::~Scorbot() {
  if (-1 != fd && close(fd))
    LOG_ERROR("Scorbot: Could not close device descriptor: %s",
              strerror(errno));
  ACL_destroy();
}

/**
 * @brief Helper function that determines if a character is a line terminator
 * for the Scorbot RX line
 * @param ch Character to determine
 * @returns 1 if a terminating char, 0 otherwise.
 */
static uint8_t is_term(char ch) {
  switch (ch) {
      // Intentional fallthroughs
    case '\r':
    case '\n':
    case '\0':
    case '>':
      return 1;
    default:
      break;
  }

  return 0;
}

/**
 * @brief Flushes the receive buffer, starting from index 0, up to index length;
 * Logs the flushed data
 * @details Assumes buffer length of ERV_TTY_BUFFER_LEN
 * @param tty_buffer The buffer to flush; intended to be the static buffer
 * storing the rx data
 * @param len Number of chars to flush
 */
static void flush_buffer(char *tty_buffer, uint16_t len) {
  if (!tty_buffer) STD_FAIL_VOID;
  if (!len) return;

  tty_buffer[ERV_TTY_BUFFER_LEN - 1] = '\0';  // Ensure end is null terminated
  if (len + 1 < ERV_TTY_BUFFER_LEN) tty_buffer[len + 1] = '\0';

  LOG_VERBOSE(0, "SCOR: %s", tty_buffer);
}

static uint16_t tval_diff_ms(struct timeval *end, struct timeval *start) {
  time_t start_ms = (start->tv_sec * 1000) + (start->tv_usec / 1000);
  time_t end_ms = (end->tv_sec * 1000) + (end->tv_usec / 1000);
  return end_ms - start_ms;
}

/**
 * @brief Helper function used to handle continuous polar pan commands during
 * polling
 * @details Continuous polar pan has to run in a loop, independent of the
 * command bus, to allow asyncrounous commands. This allows the bus to not
 * become overwhelmed.
 * @param fd File descriptor for Scorbot ER V serial device
 * @param polar_pan_cont Character describing the angular vector that the polar
 * pan is executing
 * @param manual_mode Pointer to the bool tracking the current mode of the
 * controller
 */
static void poll_polar_pan(int fd, char *polar_pan_cont,
                           struct timeval *last_start, bool *manual_mode) {
  static char last_pan_cont;

  struct timeval now;
  gettimeofday(&now, NULL);
  if (*polar_pan_cont &&
      tval_diff_ms(&now, last_start) > ERV_CONT_POLAR_PAN_TIMEOUT_MS) {
    LOG_WARN("Continuous Polar Pan timeout");
    *polar_pan_cont = '\0';
  }

  if (last_pan_cont != *polar_pan_cont) {
    // flush write buffer
    tcflush(fd, TCOFLUSH);
    if (!last_pan_cont || !*polar_pan_cont) {
      // Manual mode is toggling
      write(fd, "~", 1);
      *manual_mode = !(*manual_mode);
    }
  }

  char manual[ACL_MANUAL_MOVE_SIZE];
  if (*polar_pan_cont) {
    memset(&manual[0], *polar_pan_cont, sizeof(manual));
    write(fd, &manual, sizeof(manual));
  }

  last_pan_cont = *polar_pan_cont;
}

/**
 * @brief Helper function used to handle incoming messages from the Scorbot ER V
 * serial bus
 * @param fd File descriptor for Scorbot ER V serial device
 */
static void poll_tty_rx(int fd) {
  static clock_t last_print;
  static char buffer[ERV_TTY_BUFFER_LEN];
  static uint16_t len = 0;

  // Handle new info in buffer
  char inbox[ERV_TTY_BUFFER_LEN];
  int result = read(fd, &inbox[0], ERV_TTY_BUFFER_LEN);
  if (-1 != result) {
    for (uint16_t iter = 0; iter < result; iter++) {
      if (is_term(inbox[iter])) {
        flush_buffer(buffer, len);
        memset(buffer, 0, sizeof(buffer));
        len = 0;
        last_print = clock();
        continue;
      }

      buffer[len++] = inbox[iter];
    }
  }

  // Handle rx timeout; If timeout has occurred, flush buffer
  double delta_time_ms =
      (static_cast<float>(clock() - last_print) * 1000) / (CLOCKS_PER_SEC);
  if (len > 0 && (float)ERV_RX_TIMEOUT_MS < delta_time_ms) {
    flush_buffer(buffer, len);
    memset(buffer, 0, sizeof(buffer));
    len = 0;
    last_print = clock();
  }
}

static void execute_acl_cmd(int fd, ACL_Command *command) {
  write(fd, &command->payload[0], command->len);
  LOG_VERBOSE(4, "Sending Command: %s", &command->payload[0]);
  LOG_VERBOSE(4, "Delay_ms: %u", command->delay_ms);
  ACL_Command_init(command);
}

/**
 * @brief Helper function used to asynchronously execute ACL Commands
 * @param fd File descriptor to write commands to
 * @param cmd_buffer List of commands to execute
 */
static void poll_cmd_buffer(int fd, S_List *cmd_buffer) {
  static bool init = false;
  static uint16_t last_delay_ms = 0;
  static struct timeval last_cmd_ts;

  if (0 == cmd_buffer->len) return;
  if (!init) {
    gettimeofday(&last_cmd_ts, NULL);
    init = true;
  }

  struct timeval now;
  gettimeofday(&now, NULL);

  // Check if enough time has elapsed to overcome the delay
  uint16_t elapsed_ms = tval_diff_ms(&now, &last_cmd_ts);
  if (elapsed_ms < last_delay_ms) {
    // Wait longer
    LOG_VERBOSE(6, "elapsed_ms: %u", elapsed_ms);
    return;
  }

  // Execute next command
  S_List_Node *node = DATA_S_List_pop(cmd_buffer);
  if (!node) STD_FAIL_VOID;

  ACL_Command *cmd = DATA_LIST_GET_OBJ(node, ACL_Command, node);
  last_delay_ms = cmd->delay_ms;
  gettimeofday(&last_cmd_ts, NULL);
  execute_acl_cmd(fd, cmd);

  return;
}

void Scorbot::Poll() {
  if (-1 == fd) return;

  poll_polar_pan(fd, &polar_pan_cont, &last_start, &manual_mode);
  poll_tty_rx(fd);
  poll_cmd_buffer(fd, &cmd_buffer);
}

int Scorbot::HandShake() {
  LOG_INFO("Scorbot Recevied Handshake Command");
  return 0;
}

int Scorbot::PolarPan(API_Data_Polar_Pan *pan) {
  uint8_t iter = 0;
  char text[255];

  S_List cmd_list;
  DATA_S_List_init(&cmd_list);

  switch (oversteer) {
    case OVERSTEER_NONE:
      ACL_convert_polar_pan_direct(&cmd_list, pan);
      break;
    case OVERSTEER_IGNORE:
      ACL_convert_polar_pan_ignore(&cmd_list, pan);
      break;
    case OVERSTEER_ABORT:
      ACL_convert_polar_pan_abort(&cmd_list, pan);
      break;
    default:
      STD_FAIL;
  }

  WriteCommandQueue(&cmd_list);

  iter += sprintf(&text[iter], "Polar Pan Payload:\n");
  iter += sprintf(&text[iter], "\tΔ Azimuth: \t\t%d\n", pan->delta_azimuth);
  iter += sprintf(&text[iter], "\tΔ Altitude: \t%d\n", pan->delta_altitude);
  iter += sprintf(&text[iter], "\tDelay: \t\t%d\n", pan->delay_ms);
  iter += sprintf(&text[iter], "\tTime: \t\t%d\n", pan->time_ms);
  LOG_VERBOSE(4, "%s", text);

  return 0;
}

int Scorbot::PolarPanStart(API_Data_Polar_Pan_Start *pan) {
  uint8_t iter = 0;
  char text[255];

  iter += sprintf(&text[iter], "Polar Pan Start Payload:\n");
  iter += sprintf(&text[iter], "\tΔ Azimuth: \t%d\n", pan->delta_azimuth);
  iter += sprintf(&text[iter], "\tΔ Altitude: \t%d\n", pan->delta_altitude);
  LOG_VERBOSE(4, "%s", text);

  polar_pan_cont = ACL_get_polar_pan_continuous_vector(pan);
  gettimeofday(&last_start, NULL);

  if ('\0' == polar_pan_cont) PolarPanStop();
  return 0;
}

int Scorbot::PolarPanStop() {
  polar_pan_cont = '\0';

  S_List cmd_list;
  DATA_S_List_init(&cmd_list);
  ACL_enqueue_delay(&cmd_list, 500);
  ACL_enqueue_here_cmd(&cmd_list);
  WriteCommandQueue(&cmd_list);

  return 0;
}

int Scorbot::Home(API_Data_Home *home) {
  uint8_t iter = 0;
  char text[255];

  iter += sprintf(&text[iter], "Home Payload:\n");
  iter += sprintf(&text[iter], "\tDelay: \t\t%d", home->delay_ms);

  LOG_VERBOSE(4, "%s", text);

  S_List cmd_list;
  DATA_S_List_init(&cmd_list);
  ACL_home_sequence(&cmd_list);

  WriteCommandQueue(&cmd_list);

  return 0;
}

int Scorbot::WriteCommandQueue(S_List *cmd_list) {
  if (!cmd_list) STD_FAIL;
  if (-1 == fd) STD_FAIL;

  polar_pan_cont = '\0';
  DATA_S_List_append_list(&cmd_buffer, cmd_list);

  return 0;
}
