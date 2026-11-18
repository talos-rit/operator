#pragma once

#include <cstdint>

// Decides when the native ACL controller may be asked for telemetry
// (LISTPV POSITION).
//
// A LISTPV reply takes roughly 390 ms and cannot be cut short, so a request
// that is in flight when a jog arrives delays that jog. Polling therefore
// pauses while the arm moves (the caller only asks while the bus is idle),
// stays sparse while idle, and happens right after motion ends so the next
// move's limit checks see the new pose.
//
// Pi-Vision treats telemetry older than 1.25 s as stale, so while the bus is
// idle consecutive requests are never spaced more than kMaxIntervalMs apart.
// That cap is also the idle cadence.
class TelemetrySchedule {
 public:
  static constexpr uint32_t kMaxIntervalMs = 900;
  // After a jog ends, wait this long before the post-motion poll so a quick
  // re-press (or Pi-Vision resuming a follow) does not queue behind a LISTPV
  // reply. It may exceed the cap: Pi-Vision bounds travel between readings.
  // On Bingo (2026-11-17, same scripted jogs) restarts were blocked by a poll
  // 37% of the time with the old 1 s hold, 37% at 650 ms and 12% at 1500 ms;
  // in auto-track 58%, 48% and 34%, and only 1500 ms lost no jogs.
  static constexpr uint32_t kQuietAfterJogMs = 1500;

  // `quiet` delays the post-motion poll by kQuietAfterJogMs (jogs); queued
  // commands and aborted requests poll as soon as the bus is free.
  void motionEnded(uint64_t now_ms, bool quiet) {
    post_motion = true;
    post_motion_quiet = quiet;
    motion_end_ms = now_ms;
  }

  // A client needs a reading now. Survives the end of the jog it follows, so
  // a stop sent just before this does not bring the quiet window back.
  void pollRequested() { poll_requested = true; }

  void requested(uint64_t now_ms) {
    have_requested = true;
    last_request_ms = now_ms;
    post_motion = false;
    poll_requested = false;
  }

  bool due(uint64_t now_ms) const {
    if (!have_requested || poll_requested) return true;
    // The quiet window outranks the cap: telemetry is already stale after a
    // long jog, and a re-press inside the window must not wait for a reply.
    // Pi-Vision bounds the pose from its own commands meanwhile.
    if (post_motion && post_motion_quiet) return now_ms - motion_end_ms >= kQuietAfterJogMs;
    if (post_motion) return true;
    return now_ms - last_request_ms >= kMaxIntervalMs;
  }

 private:
  bool have_requested = false;
  bool poll_requested = false;
  uint64_t last_request_ms = 0;
  bool post_motion = false;
  bool post_motion_quiet = false;
  uint64_t motion_end_ms = 0;
};
