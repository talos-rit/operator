#include "erv_arm/erv.hpp"

#include <err.h>
#include <errno.h>
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

// The controller fd is intentionally nonblocking so the main loop can keep
// servicing TCP and serial input.  A successful write() may therefore send
// only part of an ACL command.  Keep every command contiguous on the wire and
// retry short/EAGAIN writes for a bounded interval instead of silently
// truncating them.
static bool write_serial_all(int fd, const void* data, size_t len) {
  const char* cursor = static_cast<const char*>(data);
  size_t remaining = len;
  unsigned int retries = 0;
  while (remaining > 0) {
    ssize_t written = write(fd, cursor, remaining);
    if (written > 0) {
      cursor += written;
      remaining -= static_cast<size_t>(written);
      retries = 0;
      continue;
    }
    if (written < 0 && errno == EINTR) continue;
    if ((written == 0 || errno == EAGAIN || errno == EWOULDBLOCK) &&
        retries++ < 100) {
      usleep(1000);
      continue;
    }
    LOG_ERROR("Scorbot serial write failed after %zu/%zu bytes: %s",
              len - remaining, len,
              written < 0 ? strerror(errno) : "write returned zero");
    return false;
  }
  return true;
}

static int configure_tty(int fd, bool esp) {
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
  if (ioctl(fd, TIOCMGET, &modem_bits) < 0) {
    // PTYs and some USB UARTs have no modem-control ioctl. Native ACL still
    // requires DTR/RTS; the ESP UART protocol does not.
    if (!esp || errno != ENOTTY) return -1;
  } else if (!esp) {
    modem_bits |= TIOCM_DTR | TIOCM_RTS;
    if (ioctl(fd, TIOCMSET, &modem_bits) < 0) return -1;
  }
  return tcflush(fd, TCIOFLUSH);
}

Scorbot::Scorbot(const char* dev, ControllerProfile profile) : profile(profile) {
  // Setup device
  LOG_VERBOSE(4, "Scorbot device path: %s", dev);
  strcpy(&this->dev[0], &dev[0]);
  fd = open(dev, O_RDWR | O_NOCTTY | O_NDELAY);
  if (fd < 0) {
    LOG_ERROR("Could not open Scorbot device path: %s", strerror(errno));
    raise(SIGABRT);
    return;
  }
  if (configure_tty(fd, isEsp()) < 0) {
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
  LOG_INFO("Controller profile: %s", isEsp() ? "esp-one-axis" : "native");
  // ESP firmware has no homing-completion response. Start stopped, without
  // assuming home or generating motion just because Operator was launched.
  S_List home_commands;
  DATA_S_List_init(&home_commands);
  if (isEsp()) ACL_enqueue_clrbuf_cmd(&home_commands);
  else ACL_home_sequence(&home_commands);
  writeCommandQueue(&home_commands);
  gettimeofday(&telemetry_not_before, NULL);
  write_serial_all(fd, "\r", 1);
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
                           uint16_t* telemetry_delay_ms, uint16_t jog_interval_ms,
                           bool require_prompt = true) {
  static char last_pan_cont;
  static struct timeval last_jog_byte;
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
    if (!write_serial_all(fd, "\003", 1)) return timed_out;
    *telemetry_request_pending = false;
    *direct_mode = false;
    return timed_out;
  }
  if (require_prompt && *polar_pan_cont && !*direct_mode && !*manual_mode) return timed_out;

  bool direction_changed = last_pan_cont != *polar_pan_cont;
  if (direction_changed) {
    // flush write buffer
    tcflush(fd, TCOFLUSH);
    if (!last_pan_cont || !*polar_pan_cont) {
      // Manual mode is toggling
      if (!write_serial_all(fd, "~", 1)) return timed_out;
      *manual_mode = !(*manual_mode);
      // Wait for the controller's prompt after leaving manual mode.
      *direct_mode = false;
      if (!*manual_mode) {
        gettimeofday(telemetry_not_before, NULL);
        *telemetry_delay_ms = 1000;
      }
    }
  }

  char manual[ACL_MANUAL_MOVE_SIZE];
  if (*polar_pan_cont && (direction_changed || tval_diff_ms(&now, &last_jog_byte) >= jog_interval_ms)) {
    memset(&manual[0], *polar_pan_cont, sizeof(manual));
    if (write_serial_all(fd, &manual, sizeof(manual))) last_jog_byte = now;
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
        if (sink && len > 0 &&
            (strstr(buffer, "IMPACT") != nullptr ||
             strstr(buffer, "CONTROL DISABLED") != nullptr ||
             strstr(buffer, "FAULT") != nullptr ||
             strstr(buffer, "ACL - unrecognized request") != nullptr)) {
          char fault[ERV_TTY_BUFFER_LEN + 16];
          int fault_len = snprintf(fault, sizeof(fault), "\nFAULT %.*s\n",
                                   static_cast<int>(len), buffer);
          if (fault_len > 0) {
            size_t safe_len = static_cast<size_t>(fault_len) < sizeof(fault)
                                  ? static_cast<size_t>(fault_len)
                                  : sizeof(fault) - 1;
            sink->sendTelemetry(std::string_view(fault, safe_len));
          }
        }
        flush_buffer(buffer, len);
        memset(buffer, 0, sizeof(buffer));
        len = 0;
        last_print = clock();
        continue;
      }

      if (len < ERV_TTY_BUFFER_LEN - 1) {
        buffer[len++] = inbox[iter];
      } else {
        LOG_WARN("Discarding overlong ACL RX line");
        flush_buffer(buffer, len);
        memset(buffer, 0, sizeof(buffer));
        len = 0;
      }
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

static bool execute_acl_cmd(int fd, ACL_Command* command) {
  LOG_INFO("ACL TX: %.*s", command->len, &command->payload[0]);
  bool sent = write_serial_all(fd, &command->payload[0], command->len);
  LOG_VERBOSE(4, "Sending Command: %s", &command->payload[0]);
  LOG_VERBOSE(4, "Delay_ms: %u", command->delay_ms);
  ACL_Command_init(command);
  return sent;
}

/**
 * @brief Helper function used to asynchronously execute ACL Commands
 * @param fd File descriptor to write commands to
 * @param cmd_buffer List of commands to execute
 */
bool poll_cmd_buffer(int fd, S_List* cmd_buffer) {
  static bool init = false;
  static uint16_t last_delay_ms = 0;
  static struct timeval last_cmd_ts;

  if (0 == cmd_buffer->len) return false;
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
    return false;
  }

  // Execute next command
  S_List_Node* node = DATA_S_List_pop(cmd_buffer);
  if (!node) return false;

  ACL_Command* cmd = DATA_LIST_GET_OBJ(node, ACL_Command, node);
  bool transmitted = cmd->len > 0;
  last_delay_ms = cmd->delay_ms;
  gettimeofday(&last_cmd_ts, NULL);
  bool sent = execute_acl_cmd(fd, cmd);

  return transmitted && sent;
}

void Scorbot::poll() {
  if (-1 == fd) return;
  struct timeval profile_now;
  gettimeofday(&profile_now, NULL);
  if (tval_diff_ms(&profile_now, &profile_report_at) >= 1000) {
    publishTelemetry(isEsp() ? "\nPROFILE esp-one-axis\n" : "\nPROFILE native\n");
    profile_report_at = profile_now;
  }

  if ((cmd_buffer.len == 0 || !polar_pan_cont) &&
      poll_polar_pan(fd, &polar_pan_cont, &last_start, &manual_mode,
                     &direct_mode, &telemetry_request_pending,
                     &telemetry_not_before, &telemetry_delay_ms, jog_interval_ms,
                     !isEsp())) {
    polarPanStop();
  }
  poll_tty_rx(fd, &direct_mode, &telemetry_request_pending,
              isEsp() ? nullptr : telemetrySink());
  if (isEsp()) {
    // The limited ESP ACL bridge has no prompt, LISTPV, or measured arm pose.
    // Keep queue timing and manual-mode framing, but never wait for a reply.
    if (!manual_mode && !polar_pan_cont) poll_cmd_buffer(fd, &cmd_buffer);
    return;
  }
  bool command_queue_was_active = cmd_buffer.len > 0;
  // A queued command must not interrupt a LISTPV reply or manual-mode exit.
  // Every transmitted ACL command consumes the current prompt; its response
  // supplies the next prompt before another command can use the serial bus.
  if (direct_mode && !telemetry_request_pending && !manual_mode) {
    if (poll_cmd_buffer(fd, &cmd_buffer)) direct_mode = false;
  }
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
    if (write_serial_all(fd, "LISTPV POSITION\r", 16)) {
      telemetry_request_pending = true;
      gettimeofday(&telemetry_not_before, NULL);
      telemetry_delay_ms = ERV_TELEMETRY_INTERVAL_MS;
    }
  }
}

int Scorbot::handShake() {
  LOG_INFO("Scorbot Received Handshake Command");
  return 0;
}

int Scorbot::polarPan(API::PolarPan* pan) {
  if (isEsp()) {
    LOG_WARN("ESP one-axis: discrete position moves unsupported; use continuous base jog");
    polarPanStop();
    return -1;
  }
  uint8_t iter = 0;
  char text[255];

  S_List cmd_list;
  DATA_S_List_init(&cmd_list);

  // Only position-based commands use DELTA. Refresh it here after manual
  // jogging, with the same prompt arbitration and quiet period as before.
  // A stop/re-press of a manual jog must not wait for this reference work.
  if (manual_reference_dirty) {
    ACL_generate_enqueue_defp_cmd(&cmd_list);
    ACL_enqueue_delay(&cmd_list, 500);
    // The polar conversion below already prepends HERE DELTA.
    manual_reference_dirty = false;
  }

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
  if (isEsp() && pan->delta_altitude != 0) {
    LOG_WARN("ESP one-axis: only base motion is supported");
    polarPanStop();
    return -1;
  }
  tracking_jog = false;
  jog_interval_ms = 10;
  uint8_t iter = 0;
  char text[255];

  iter += sprintf(&text[iter], "Polar Pan Start Payload:\n");
  iter += sprintf(&text[iter], "\tΔ Azimuth: \t%d\n", pan->delta_azimuth);
  iter += sprintf(&text[iter], "\tΔ Altitude: \t%d\n", pan->delta_altitude);
  LOG_VERBOSE(4, "%s", text);

  polar_pan_cont = ACL_get_polar_pan_continuous_vector(pan);
  if (!isEsp() && polar_pan_cont) manual_reference_dirty = true;
  gettimeofday(&last_start, NULL);

  if ('\0' == polar_pan_cont) polarPanStop();
  return 0;
}

int Scorbot::polarPanStop() {
  polar_pan_cont = '\0';
  if (isEsp()) {
    S_List commands;
    DATA_S_List_init(&commands);
    ACL_enqueue_clrbuf_cmd(&commands);
    return writeCommandQueue(&commands);
  }
  // poll() exits manual mode immediately. Leave DELTA dirty until an explicit
  // position command needs it; reference updates must not delay the next hold.
  return 0;
}

int Scorbot::executeHardwareOperation(API::HardwareOperation* operation) {
  if (!operation) STD_FAIL;
  if (isEsp() && operation->subcommand != static_cast<uint8_t>(API::HardwareOperationID::TrackingJog) &&
      operation->subcommand != static_cast<uint8_t>(API::HardwareOperationID::JointJogStop)) {
    LOG_WARN("ESP one-axis: hardware operation %u unsupported by firmware", operation->subcommand);
    // An unsupported operation must not leave an earlier jog running.
    polarPanStop();
    return -1;
  }

  switch (static_cast<API::HardwareOperationID>(operation->subcommand)) {
    case API::HardwareOperationID::TrackingJog: {
      tracking_jog = true;
      auto* jog = reinterpret_cast<API::TrackingJog*>(operation + 1);
      if (isEsp() && jog->altitude != 0) {
        LOG_WARN("ESP one-axis: altitude tracking unsupported");
        polarPanStop();
        return -1;
      }
      API::PolarPanStart pan{jog->azimuth, jog->altitude};
      polar_pan_cont = ACL_get_polar_pan_continuous_vector(&pan);
      jog_interval_ms = jog->interval_ms;
      // Firmware's key timeout is 100 ms, including when callers request a
      // slower native-controller repeat interval.
      if (isEsp() && jog_interval_ms > 50) jog_interval_ms = 50;
      gettimeofday(&last_start, NULL);
      return 0;
    }
    case API::HardwareOperationID::JointJogStart: {
      tracking_jog = false;
      jog_interval_ms = 10;
      auto* jog = reinterpret_cast<API::JointJogStart*>(operation + 1);
      char vector = ACL_get_joint_jog_vector(jog->axis, jog->direction);
      if ('\0' == vector) STD_FAIL;
      if (polar_pan_cont != vector) LOG_INFO("Joint jog start: axis=%u direction=%d prompt_ready=%d queued=%u", jog->axis, jog->direction, direct_mode, cmd_buffer.len);
      manual_reference_dirty = true;
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
  if (isEsp()) ACL_generate_enqueue_home_cmd(&cmd_list);
  else ACL_home_sequence(&cmd_list);

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
