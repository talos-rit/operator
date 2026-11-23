#pragma once

#include <cstdint>
#include <cstdio>
#include <cstring>

// Health of the native ACL serial link, and recovery from a lost prompt.
//
// Operator only writes to the controller after a '>' prompt. One corrupted or
// dropped '>' used to leave it waiting forever: no telemetry, no commands, no
// jogs (Maker Faire, 2026-11-21, a loose serial connector). SerialLink notices
// when an expected prompt is overdue and says how to ask for a new one, and it
// counts how much of the controller's output arrives damaged so the link's
// quality can be monitored (Gaia reads the periodic "Serial link:" log line).
class SerialLink {
 public:
  // An ordinary command answers within a few hundred ms. HOME and MOVED only
  // answer when the motion finishes, so they get a much longer allowance.
  static constexpr uint64_t kPromptTimeoutMs = 3000;
  static constexpr uint64_t kLongCommandTimeoutMs = 60000;
  // After Operator sends '~' the controller needs a moment to announce the
  // mode change; until then its last announcement is stale.
  static constexpr uint64_t kModeSettleMs = 500;
  static constexpr uint64_t kReportIntervalMs = 10000;
  static constexpr uint32_t kQuickAttempts = 4;
  static constexpr uint64_t kSlowRetryMs = 15000;

  enum class Recovery { None, SendReturn, SendTilde };

  struct Window {
    uint32_t polls = 0;      // LISTPV POSITION requests sent
    uint32_t echo_ok = 0;    // ...whose echo came back intact
    uint32_t joint = 0;      // joint-position lines received
    uint32_t framing = 0;    // bytes received with a framing error ('\0')
    uint32_t recoveries = 0; // prompt recoveries attempted
  };

  // ---- inputs ----

  void onByte(uint8_t byte, uint64_t now_ms) {
    last_rx_ms = now_ms;
    heard_anything = true;
    // In raw mode Linux delivers a byte received with a framing or parity
    // error as '\0'. The controller never sends NUL itself.
    if (byte == '\0') window.framing++;
  }

  void onPrompt() {
    prompt_seen = true;
    long_command = false;
    attempts = 0;
  }

  void onLine(const char* line, size_t len) {
    if (!len) return;
    if (len == kEchoLen && strncmp(line, kEcho, kEchoLen) == 0) window.echo_ok++;
    // The controller announces manual mode on entry ("MANUAL MODE!", then
    // " JOINT MODE." or " XYZ MODE.") and on exit ("EXIT MANUAL MODE...").
    if (contains(line, len, "EXIT MANUAL MODE")) {
      controller_manual = false;
      announced_since_tilde = true;
    } else if (contains(line, len, "MANUAL MODE!") || contains(line, len, "JOINT MODE.") ||
               contains(line, len, "XYZ MODE.")) {
      controller_manual = true;
      announced_since_tilde = true;
    }
  }

  void onJointLine() { window.joint++; }
  void onPollSent() { window.polls++; }

  void onCommandSent(const char* payload, size_t len) {
    long_command = startsWith(payload, len, "HOME") || startsWith(payload, len, "MOVED");
  }

  // `recovery`: Operator sent this '~' without knowing the controller's mode
  // (a prompt recovery, or leaving a stray manual mode).
  void onTildeSent(uint64_t now_ms, bool recovery = false) {
    last_tilde_ms = now_ms;
    announced_since_tilde = false;
    recovery_tilde = recovery;
  }

  // ---- decisions ----

  // The controller's last mode announcement is current: it has announced
  // since Operator's last '~', or that '~' was long enough ago.
  bool modeSettled(uint64_t now_ms) const {
    return announced_since_tilde || now_ms - last_tilde_ms >= kModeSettleMs;
  }

  // ACL text (commands, LISTPV) may be sent. In manual mode the controller
  // would read it as jog keys, so it waits until the mode is known to be
  // direct.
  bool mayTypeCommands(uint64_t now_ms) const {
    return modeSettled(now_ms) && !controller_manual;
  }

  // A jog may start (send its entry '~'). Not while the controller says it is
  // in manual mode, or while a recovery '~' may have put it there: the entry
  // '~' would take it out instead and the jog keys would become text. An
  // ordinary jog exit doesn't hold up the next jog.
  bool mayStartJog(uint64_t now_ms) const {
    if (recovery_tilde && !modeSettled(now_ms)) return false;
    return !controllerManual(now_ms);
  }

  // The controller says it is in manual mode. With no jog running, Operator
  // should leave it.
  bool controllerManual(uint64_t now_ms) const {
    return modeSettled(now_ms) && controller_manual;
  }

  // `waiting`: Operator is waiting for a prompt. Returns what to send when
  // the prompt is overdue. Any output from the controller restarts the clock,
  // so a long reply or a homing announcement never trips it.
  Recovery poll(bool waiting, uint64_t now_ms) {
    if (!waiting) {
      was_waiting = false;
      return Recovery::None;
    }
    if (!was_waiting) {
      was_waiting = true;
      waiting_since_ms = now_ms;
    }
    uint64_t quiet_since = waiting_since_ms;
    if (last_rx_ms > quiet_since) quiet_since = last_rx_ms;
    if (last_attempt_ms > quiet_since) quiet_since = last_attempt_ms;
    uint64_t limit = long_command ? kLongCommandTimeoutMs : kPromptTimeoutMs;
    // A controller that stays silent (unplugged, powered off) is retried less
    // often once the first few attempts have failed.
    if (attempts >= kQuickAttempts) limit = kSlowRetryMs;
    if (now_ms - quiet_since < limit) return Recovery::None;

    last_attempt_ms = now_ms;
    window.recoveries++;
    long_command = false;
    // A bare CR answers with a prompt in direct mode (and ends a half-typed
    // line). If the controller is silently in manual mode, CR does nothing
    // there, so every other attempt toggles with '~'. Entering manual mode by
    // mistake is announced, and Scorbot then leaves it again.
    bool tilde = controller_manual || attempts % 2 == 1;
    attempts++;
    return tilde ? Recovery::SendTilde : Recovery::SendReturn;
  }

  // A "Serial link:" summary every kReportIntervalMs, or nullptr.
  const char* report(uint64_t now_ms) {
    if (!reporting) {
      reporting = true;
      report_at_ms = now_ms;
    }
    if (now_ms - report_at_ms < kReportIntervalMs) return nullptr;
    uint32_t damaged = window.polls > window.echo_ok ? window.polls - window.echo_ok : 0;
    snprintf(text, sizeof(text),
             "Serial link: polls=%u damaged=%u joint=%u framing=%u recoveries=%u quiet_ms=%llu",
             window.polls, damaged, window.joint, window.framing, window.recoveries,
             static_cast<unsigned long long>(now_ms - (heard_anything ? last_rx_ms : report_at_ms)));
    window = Window{};
    report_at_ms = now_ms;
    return text;
  }

  bool takePrompt() {
    bool seen = prompt_seen;
    prompt_seen = false;
    return seen;
  }

  uint32_t attemptsSincePrompt() const { return attempts; }

 private:
  static constexpr const char* kEcho = "LISTPV POSITION";
  static constexpr size_t kEchoLen = 15;

  static bool contains(const char* s, size_t len, const char* token) {
    size_t n = strlen(token);
    for (size_t i = 0; i + n <= len; i++)
      if (memcmp(s + i, token, n) == 0) return true;
    return false;
  }
  static bool startsWith(const char* s, size_t len, const char* token) {
    size_t n = strlen(token);
    return len >= n && memcmp(s, token, n) == 0;
  }

  Window window;
  bool controller_manual = false;
  bool announced_since_tilde = true;  // no '~' sent yet
  bool recovery_tilde = false;
  bool reporting = false;
  bool heard_anything = false;
  bool long_command = false;
  bool prompt_seen = false;
  uint32_t attempts = 0;
  uint64_t last_rx_ms = 0;
  uint64_t last_tilde_ms = 0;
  uint64_t last_attempt_ms = 0;
  bool was_waiting = false;
  uint64_t waiting_since_ms = 0;
  uint64_t report_at_ms = 0;
  char text[160];
};
