#include "erv_arm/erv.hpp"

#include <err.h>
#include <fcntl.h>
#include <signal.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <termios.h>
#include <unistd.h>

#include "acl/acl.hpp"
#include "log/log.hpp"
#include "util/comm.hpp"

#define LOG_CONSOLE_THRESHOLD_THIS LOG_THRESHOLD_DEFAULT
#define LOG_FILE_THRESHOLD_THIS LOG_THRESHOLD_MAX

#define ERV_DEFAULT_COMMAND_DELAY 200000
#define ERV_TTY_BUFFER_LEN 127

#define ERV_CLOCK CLOCK_REALTIME

static int configure_tty(int fd) {
  struct termios settings;
  if (tcgetattr(fd, &settings) < 0) return -1;
  cfmakeraw(&settings);
  cfsetispeed(&settings, B9600);
  cfsetospeed(&settings, B9600);
  settings.c_cflag |= (CS8 | CLOCAL | CREAD);
  // Keep DTR asserted across a close/reopen and eliminate stale controller
  // bytes before the startup HOME/telemetry sequence begins.
  settings.c_cflag &= ~(PARENB | CSTOPB | CRTSCTS | HUPCL);
  settings.c_iflag |= (IXON | IXOFF);
  if (tcsetattr(fd, TCSANOW, &settings) < 0) return -1;
  int modem_bits;
  if (ioctl(fd, TIOCMGET, &modem_bits) < 0) return -1;
  modem_bits |= TIOCM_DTR | TIOCM_RTS;
  if (ioctl(fd, TIOCMSET, &modem_bits) < 0) return -1;
  return tcflush(fd, TCIOFLUSH);
}

Scorbot::Scorbot(const char* dev) {
  // Setup device
  LOG_VERBOSE(4, "Scorbot device path: %s", dev);
  strcpy(&this->dev[0], &dev[0]);
  fd = open(dev, O_RDWR | O_NOCTTY | O_NDELAY);
  if (fd < 0) {
    LOG_ERROR("Could not open Scorbot device path: %s", strerror(errno));
    raise(SIGABRT);
    return;
  }
  if (configure_tty(fd) < 0) {
    LOG_ERROR("Could not configure Scorbot serial device: %s", strerror(errno));
    close(fd);
    fd = -1;
    raise(SIGABRT);
    return;
  }

  // Setup config
  polar_pan_cont = '\0';
  manual_mode = false;
  direct_mode = false;
  telemetry_request_pending = false;
  telemetry_delay_ms = ERV_HOME_SETTLE_MS;
  oversteer = OversteerConfig::Abort;

  // Setup ACL
  ACL_init();
  DATA_S_List_init(&cmd_buffer);
  gettimeofday(&last_start, NULL);

  ACL_flush_tx(&cmd_buffer);
  // A home cycle establishes the encoder-zero reference required for telemetry.
  S_List home_commands;
  DATA_S_List_init(&home_commands);
  ACL_home_sequence(&home_commands);
  writeCommandQueue(&home_commands);
  gettimeofday(&telemetry_not_before, NULL);
  write(fd, "\r", 1);
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
static void flush_buffer(char* tty_buffer, uint16_t len) {
  if (!tty_buffer) STD_FAIL_VOID;
  if (!len) return;

  tty_buffer[ERV_TTY_BUFFER_LEN - 1] = '\0';  // Ensure end is null terminated
  if (len + 1 < ERV_TTY_BUFFER_LEN) tty_buffer[len + 1] = '\0';

  LOG_VERBOSE(0, "SCOR: %s", tty_buffer);
  LOG_INFO("ACL RX: %s", tty_buffer);
}

static uint16_t tval_diff_ms(struct timeval* end, struct timeval* start) {
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
static bool poll_polar_pan(int fd, char* polar_pan_cont,
                           struct timeval* last_start, bool* manual_mode,
                           bool* direct_mode, bool* telemetry_request_pending,
                           struct timeval* telemetry_not_before,
                           uint16_t* telemetry_delay_ms) {
  static char last_pan_cont;
  bool timed_out = false;

  struct timeval now;
  gettimeofday(&now, NULL);
  if (*polar_pan_cont &&
      tval_diff_ms(&now, last_start) > ERV_CONT_POLAR_PAN_TIMEOUT_MS) {
    LOG_WARN("Continuous Polar Pan timeout");
    *polar_pan_cont = '\0';
    timed_out = true;
  }

  if (*polar_pan_cont && *telemetry_request_pending) {
    write(fd, "\003", 1);
    *telemetry_request_pending = false;
    *direct_mode = false;
    return timed_out;
  }
  if (*polar_pan_cont && !*direct_mode) return timed_out;

  if (last_pan_cont != *polar_pan_cont) {
    // flush write buffer
    tcflush(fd, TCOFLUSH);
    if (!last_pan_cont || !*polar_pan_cont) {
      // Manual mode is toggling
      write(fd, "~", 1);
      *manual_mode = !(*manual_mode);
      *direct_mode = !(*manual_mode);
      if (!*manual_mode) {
        gettimeofday(telemetry_not_before, NULL);
        *telemetry_delay_ms = 1000;
      }
    }
  }

  char manual[ACL_MANUAL_MOVE_SIZE];
  if (*polar_pan_cont) {
    memset(&manual[0], *polar_pan_cont, sizeof(manual));
    write(fd, &manual, sizeof(manual));
  }

  last_pan_cont = *polar_pan_cont;
  return timed_out;
}

/**
 * @brief Helper function used to handle incoming messages from the Scorbot ER V
 * serial bus
 * @param fd File descriptor for Scorbot ER V serial device
 */
static void poll_tty_rx(int fd, bool* direct_mode,
                        bool* telemetry_request_pending, TelemetrySink* sink) {
  static clock_t last_print;
  static char buffer[ERV_TTY_BUFFER_LEN];
  static uint16_t len = 0;

  // Handle new info in buffer
  char inbox[ERV_TTY_BUFFER_LEN];
  int result = read(fd, &inbox[0], ERV_TTY_BUFFER_LEN);
  if (-1 != result) {
    for (uint16_t iter = 0; iter < result; iter++) {
      if (is_term(inbox[iter])) {
        if (inbox[iter] == '>') {
          *direct_mode = true;
          *telemetry_request_pending = false;
        }
        char* cursor = buffer;
        long values[11];
        bool encoders = len > 0;
        for (int axis = 0; axis < 11 && encoders; axis++) {
          char* end = nullptr;
          values[axis] = strtol(cursor, &end, 10);
          encoders = end != cursor;
          cursor = end;
        }
        if (encoders && sink) {
          char telemetry[160];
          int telemetry_len = snprintf(
              telemetry, sizeof(telemetry),
              "\nTEL %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld\n", values[0],
              values[1], values[2], values[3], values[4], values[5], values[6],
              values[7], values[8], values[9], values[10]);
          if (telemetry_len > 0) sink->sendTelemetry(
              std::string_view(telemetry, telemetry_len));
        }
        long base, shoulder, elbow, wrist_pitch, wrist_roll;
        if (sink && sscanf(buffer, " %*d:%ld %*d:%ld %*d:%ld %*d:%ld %*d:%ld",
                           &base, &shoulder, &elbow, &wrist_pitch,
                           &wrist_roll) == 5) {
          char telemetry[112];
          int telemetry_len = snprintf(telemetry, sizeof(telemetry),
              "\nTELP %ld %ld %ld %ld %ld\n", base, shoulder, elbow,
              wrist_pitch, wrist_roll);
          if (telemetry_len > 0)
            sink->sendTelemetry(std::string_view(telemetry, telemetry_len));
        }
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

static void execute_acl_cmd(int fd, ACL_Command* command) {
  LOG_INFO("ACL TX: %.*s", command->len, &command->payload[0]);
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
static void poll_cmd_buffer(int fd, S_List* cmd_buffer) {
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
  S_List_Node* node = DATA_S_List_pop(cmd_buffer);
  if (!node) STD_FAIL_VOID;

  ACL_Command* cmd = DATA_LIST_GET_OBJ(node, ACL_Command, node);
  last_delay_ms = cmd->delay_ms;
  gettimeofday(&last_cmd_ts, NULL);
  execute_acl_cmd(fd, cmd);

  return;
}

void Scorbot::poll() {
  if (-1 == fd) return;

  if (poll_polar_pan(fd, &polar_pan_cont, &last_start, &manual_mode,
                     &direct_mode, &telemetry_request_pending,
                     &telemetry_not_before, &telemetry_delay_ms)) {
    polarPanStop();
  }
  poll_tty_rx(fd, &direct_mode, &telemetry_request_pending, telemetrySink());
  bool command_queue_was_active = cmd_buffer.len > 0;
  poll_cmd_buffer(fd, &cmd_buffer);
  struct timeval now;
  gettimeofday(&now, NULL);
  if (command_queue_was_active) {
    // The controller has no command framing beyond its prompt.  Do not append
    // a telemetry query to the final MOVE command while it is still executing.
    telemetry_not_before = now;
    telemetry_delay_ms = 500;
  }
  if (direct_mode && !telemetry_request_pending && !manual_mode &&
      !polar_pan_cont && cmd_buffer.len == 0 &&
      tval_diff_ms(&now, &telemetry_not_before) >= telemetry_delay_ms) {
    write(fd, "LISTPV POSITION\r", 16);
    telemetry_request_pending = true;
    gettimeofday(&telemetry_not_before, NULL);
    telemetry_delay_ms = ERV_TELEMETRY_INTERVAL_MS;
  }
}

int Scorbot::handShake() {
  LOG_INFO("Scorbot Received Handshake Command");
  return 0;
}

int Scorbot::polarPan(API::PolarPan* pan) {
  uint8_t iter = 0;
  char text[255];

  S_List cmd_list;
  DATA_S_List_init(&cmd_list);

  switch (oversteer) {
    case OversteerConfig::None:
      ACL_convert_polar_pan_direct(&cmd_list, pan);
      break;
    case OversteerConfig::Ignore:
      ACL_convert_polar_pan_ignore(&cmd_list, pan);
      break;
    case OversteerConfig::Abort:
      ACL_convert_polar_pan_abort(&cmd_list, pan);
      break;
    default:
      STD_FAIL;
  }

  writeCommandQueue(&cmd_list);

  iter += sprintf(&text[iter], "Polar Pan Payload:\n");
  iter += sprintf(&text[iter], "\tΔ Azimuth: \t\t%d\n", pan->delta_azimuth);
  iter += sprintf(&text[iter], "\tΔ Altitude: \t%d\n", pan->delta_altitude);
  iter += sprintf(&text[iter], "\tDelay: \t\t%d\n", pan->delay_ms);
  iter += sprintf(&text[iter], "\tTime: \t\t%d\n", pan->time_ms);
  LOG_VERBOSE(4, "%s", text);

  return 0;
}

int Scorbot::polarPanStart(API::PolarPanStart* pan) {
  uint8_t iter = 0;
  char text[255];

  iter += sprintf(&text[iter], "Polar Pan Start Payload:\n");
  iter += sprintf(&text[iter], "\tΔ Azimuth: \t%d\n", pan->delta_azimuth);
  iter += sprintf(&text[iter], "\tΔ Altitude: \t%d\n", pan->delta_altitude);
  LOG_VERBOSE(4, "%s", text);

  polar_pan_cont = ACL_get_polar_pan_continuous_vector(pan);
  gettimeofday(&last_start, NULL);

  if ('\0' == polar_pan_cont) polarPanStop();
  return 0;
}

int Scorbot::polarPanStop() {
  polar_pan_cont = '\0';

  S_List cmd_list;
  DATA_S_List_init(&cmd_list);
  ACL_enqueue_delay(&cmd_list, 500);
  ACL_enqueue_here_cmd(&cmd_list);
  writeCommandQueue(&cmd_list);

  return 0;
}

int Scorbot::executeHardwareOperation(API::HardwareOperation* operation) {
  if (!operation) STD_FAIL;

  switch (static_cast<API::HardwareOperationID>(operation->subcommand)) {
    case API::HardwareOperationID::JointJogStart: {
      auto* jog = reinterpret_cast<API::JointJogStart*>(operation + 1);
      char vector = ACL_get_joint_jog_vector(jog->axis, jog->direction);
      if ('\0' == vector) STD_FAIL;
      polar_pan_cont = vector;
      gettimeofday(&last_start, NULL);
      return 0;
    }
    case API::HardwareOperationID::JointJogStop:
      return polarPanStop();
    case API::HardwareOperationID::JointMoveRelative: {
      auto* move = reinterpret_cast<API::JointMoveRelative*>(operation + 1);
      LOG_INFO("Joint target counts: shoulder=%d elbow=%d pitch=%d",
               move->shoulder, move->elbow, move->wrist_pitch);
      S_List commands;
      DATA_S_List_init(&commands);
      ACL_enqueue_here_cmd(&commands);
      ACL_enqueue_shift_counts_cmd(&commands, 2, move->shoulder);
      ACL_enqueue_shift_counts_cmd(&commands, 3, move->elbow);
      ACL_enqueue_shift_counts_cmd(&commands, 4, move->wrist_pitch);
      // Use the discrete sequence that is already physically verified.
      ACL_enqueue_clrbuf_cmd(&commands);
      ACL_enqueue_move_cmd(&commands);
      writeCommandQueue(&commands);
      return 0;
    }
    case API::HardwareOperationID::EnableControl: {
      S_List commands;
      DATA_S_List_init(&commands);
      // CON re-enables controller servo/control; it does not clear an arbitrary fault.
      ACL_enqueue_enable_control_cmd(&commands);
      writeCommandQueue(&commands);
      return 0;
    }
    case API::HardwareOperationID::SetSpeedPercent: {
      auto* speed = reinterpret_cast<API::SpeedPercent*>(operation + 1);
      if (speed->percent < 1 || speed->percent > 100) STD_FAIL;
      S_List commands;
      DATA_S_List_init(&commands);
      ACL_enqueue_speed_percent_cmd(&commands, speed->percent);
      writeCommandQueue(&commands);
      return 0;
    }
    default:
      STD_FAIL;
  }
}

int Scorbot::home(API::Home* home) {
  uint8_t iter = 0;
  char text[255];

  iter += sprintf(&text[iter], "Home Payload:\n");
  iter += sprintf(&text[iter], "\tDelay: \t\t%d", home->delay_ms);

  LOG_VERBOSE(4, "%s", text);

  S_List cmd_list;
  DATA_S_List_init(&cmd_list);
  ACL_home_sequence(&cmd_list);

  writeCommandQueue(&cmd_list);

  return 0;
}

int Scorbot::writeCommandQueue(S_List* cmd_list) {
  if (!cmd_list) STD_FAIL;
  if (-1 == fd) STD_FAIL;

  polar_pan_cont = '\0';
  DATA_S_List_append_list(&cmd_buffer, cmd_list);

  return 0;
}
