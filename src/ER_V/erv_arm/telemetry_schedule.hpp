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
// Pi-Vision stops enforcing joint limits once telemetry is older than 1.25 s,
// so consecutive requests are never spaced more than kMaxIntervalMs apart
// whenever the bus is free. That cap is also the idle cadence.
class TelemetrySchedule {
 public:
  static constexpr uint32_t kMaxIntervalMs = 900;
  // Pi-Vision re-plans ~120 ms after each tracking burst. Waiting slightly
  // longer than that keeps a post-burst poll from landing in front of the next
  // burst; continuous tracking still gets polled once kMaxIntervalMs is hit.
  static constexpr uint32_t kTrackingQuietMs = 250;

  void motionEnded(uint64_t now_ms, bool tracking) {
    post_motion = true;
    post_motion_tracking = tracking;
    motion_end_ms = now_ms;
  }

  void requested(uint64_t now_ms) {
    have_requested = true;
    last_request_ms = now_ms;
    post_motion = false;
  }

  bool due(uint64_t now_ms) const {
    if (!have_requested) return true;
    if (now_ms - last_request_ms >= kMaxIntervalMs) return true;
    if (!post_motion) return false;
    return !post_motion_tracking || now_ms - motion_end_ms >= kTrackingQuietMs;
  }

 private:
  bool have_requested = false;
  uint64_t last_request_ms = 0;
  bool post_motion = false;
  bool post_motion_tracking = false;
  uint64_t motion_end_ms = 0;
};
