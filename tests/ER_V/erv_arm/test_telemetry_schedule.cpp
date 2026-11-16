#include <CppUTest/TestHarness.h>
#include <endian.h>
#include <fcntl.h>
#include <sys/time.h>
#include <unistd.h>
#include <cstring>
#include <string>
#include <vector>
#include "erv_arm/erv.hpp"
#include "erv_arm/telemetry_schedule.hpp"

// Pi-Vision skips joint-limit checks once telemetry is older than this.
static constexpr uint32_t PI_VISION_TELEMETRY_TIMEOUT_MS = 1250;

TEST_GROUP(TelemetrySchedule) {};

TEST(TelemetrySchedule, CapLeavesRoomForASlowReplyUnderPiVisionTimeout) {
  // Telemetry age at Pi-Vision tracks the request-to-request interval; leave
  // margin for LISTPV reply-time jitter (~390 ms nominal).
  CHECK(TelemetrySchedule::kMaxIntervalMs + 300 <= PI_VISION_TELEMETRY_TIMEOUT_MS);
  CHECK(TelemetrySchedule::kQuietAfterJogMs < TelemetrySchedule::kMaxIntervalMs);
}

TEST(TelemetrySchedule, FirstRequestIsDueImmediately) {
  TelemetrySchedule schedule;
  CHECK_TRUE(schedule.due(0));
}

TEST(TelemetrySchedule, IdleRequestsAreSpacedByTheCap) {
  TelemetrySchedule schedule;
  schedule.requested(1000);
  CHECK_FALSE(schedule.due(1000));
  CHECK_FALSE(schedule.due(1000 + TelemetrySchedule::kMaxIntervalMs - 1));
  CHECK_TRUE(schedule.due(1000 + TelemetrySchedule::kMaxIntervalMs));
}

TEST(TelemetrySchedule, QueuedMotionEndMakesPollDueImmediately) {
  TelemetrySchedule schedule;
  schedule.requested(1000);
  schedule.motionEnded(1100, false);
  CHECK_TRUE(schedule.due(1100));
  schedule.requested(1100);
  CHECK_FALSE(schedule.due(1150));  // One post-motion poll, then idle cadence.
}

TEST(TelemetrySchedule, JogEndWaitsForQuietWindow) {
  TelemetrySchedule schedule;
  schedule.requested(1000);
  schedule.motionEnded(1100, true);
  CHECK_FALSE(schedule.due(1100 + TelemetrySchedule::kQuietAfterJogMs - 1));
  CHECK_TRUE(schedule.due(1100 + TelemetrySchedule::kQuietAfterJogMs));
}

TEST(TelemetrySchedule, QuietWindowOutranksTheCap) {
  TelemetrySchedule schedule;
  schedule.requested(0);
  // A jog longer than the cap: its end still waits out the quiet window.
  uint64_t end = TelemetrySchedule::kMaxIntervalMs + 300;
  schedule.motionEnded(end, true);
  CHECK_FALSE(schedule.due(end + TelemetrySchedule::kQuietAfterJogMs - 1));
  CHECK_TRUE(schedule.due(end + TelemetrySchedule::kQuietAfterJogMs));
  schedule.requested(end + TelemetrySchedule::kQuietAfterJogMs);
  CHECK_FALSE(schedule.due(end + TelemetrySchedule::kQuietAfterJogMs + 1));
}

// ---------------------------------------------------------------------------
// End to end over a PTY: the simulated controller answers every command,
// manual-mode toggle and interrupt with a prompt. The ioctl wrapper for PTYs
// lives in test_native_stop.cpp.
//
// Upper timing bounds are loose so a loaded build machine does not flake; the
// lower bounds carry the behaviour under test.

namespace {
constexpr long kSlackMs = 200;

struct Chunk {
  long at_ms;
  std::string bytes;
};

long now_ms() {
  struct timeval tv;
  gettimeofday(&tv, nullptr);
  return tv.tv_sec * 1000L + tv.tv_usec / 1000;
}

std::vector<long> times_of(const std::vector<Chunk>& chunks, const char* token) {
  std::vector<long> times;
  for (const auto& chunk : chunks)
    for (size_t p = chunk.bytes.find(token); p != std::string::npos;
         p = chunk.bytes.find(token, p + 1))
      times.push_back(chunk.at_ms);
  return times;
}
}  // namespace

TEST_GROUP(TelemetryPolling) {};

TEST(TelemetryPolling, PausesDuringMotionPollsAfterAndIdlesAtTheCap) {
  int master = posix_openpt(O_RDWR | O_NOCTTY | O_NONBLOCK);
  CHECK(master >= 0);
  CHECK_EQUAL(0, grantpt(master));
  CHECK_EQUAL(0, unlockpt(master));
  Subscriber subscriber;
  CHECK(subscriber.start());
  {
    Scorbot robot(ptsname(master));
    robot.registerSubscriber(&subscriber);
    std::vector<Chunk> log;
    auto pump = [&](int milliseconds) {
      long until = now_ms() + milliseconds;
      while (now_ms() < until) {
        static_cast<Arm&>(robot).poll();
        char bytes[1024];
        ssize_t n;
        while ((n = read(master, bytes, sizeof(bytes))) > 0) {
          log.push_back({now_ms(), std::string(bytes, n)});
          if (memchr(bytes, '\r', n) || memchr(bytes, '~', n) || memchr(bytes, '\003', n))
            CHECK_EQUAL(1, write(master, ">", 1));
        }
        usleep(5000);
      }
    };
    unsigned sequence = 0;
    auto send = [&](API::CommandID command, const void* payload, size_t length) {
      auto* buffer = subscriber.dequeueBuffer(Sub_Queue::Free);
      CHECK(buffer != nullptr);
      API::DataHeader header{htobe32(++sequence), 0,
          htobe16(static_cast<uint16_t>(command)), htobe16(static_cast<uint16_t>(length))};
      memcpy(buffer->body.data(), &header, sizeof(header));
      if (length) memcpy(buffer->body.data() + sizeof(header), payload, length);
      buffer->len = sizeof(header) + length + 1;
      CHECK(subscriber.enqueueBuffer(Sub_Queue::Command, buffer));
      return robot.processCommand();
    };
    struct __attribute__((packed)) Hardware {
      API::HardwareOperation operation;
      API::JointJogStart jog;
    };
    struct __attribute__((packed)) Tracking {
      API::HardwareOperation operation;
      API::TrackingJog jog;
    };

    // Idle: after the startup HOME queue drains, polls run at the cap.
    pump(1200);
    log.clear();
    pump(3000);
    std::vector<long> idle = times_of(log, "LISTPV");
    CHECK(idle.size() >= 3);
    for (size_t i = 1; i < idle.size(); ++i) {
      long gap = idle[i] - idle[i - 1];
      CHECK_TEXT(gap >= static_cast<long>(TelemetrySchedule::kMaxIntervalMs) - 5,
                 std::to_string(gap).c_str());
      CHECK_TEXT(gap <= static_cast<long>(TelemetrySchedule::kMaxIntervalMs) + kSlackMs,
                 std::to_string(gap).c_str());
    }

    // Manual jog: no polling between entering and leaving manual mode, then
    // one poll once the quiet window has passed.
    pump(100);  // Let the post-idle poll's prompt come back.
    log.clear();
    Hardware joint{{static_cast<uint8_t>(API::HardwareOperationID::JointJogStart), 0}, {2, 1}};
    long jog_until = now_ms() + 1200;  // Longer than the cap.
    while (now_ms() < jog_until) {
      CHECK(send(API::CommandID::ExecuteHardwareOperation, &joint, sizeof(joint)));
      pump(100);
    }
    CHECK(send(API::CommandID::PolarPanStop, nullptr, 0));
    pump(TelemetrySchedule::kQuietAfterJogMs + kSlackMs);
    std::vector<long> toggles = times_of(log, "~");
    CHECK_EQUAL(2, toggles.size());
    for (long t : times_of(log, "LISTPV")) CHECK(t < toggles[0] || t >= toggles[1]);
    std::vector<long> after_jog;
    for (long t : times_of(log, "LISTPV"))
      if (t >= toggles[1]) after_jog.push_back(t);
    CHECK_EQUAL(1, after_jog.size());
    long jog_held = after_jog[0] - toggles[1];
    CHECK_TEXT(jog_held >= static_cast<long>(TelemetrySchedule::kQuietAfterJogMs) - 5,
               std::to_string(jog_held).c_str());
    CHECK_TEXT(jog_held <= static_cast<long>(TelemetrySchedule::kQuietAfterJogMs) + kSlackMs,
               std::to_string(jog_held).c_str());

    // Tracking jog: same quiet window, so a resumed follow is not stuck behind
    // an uninterruptible LISTPV reply. Start right after an idle poll so the
    // cap cannot fire first.
    log.clear();
    long poll_wait_until = now_ms() + 2 * static_cast<long>(TelemetrySchedule::kMaxIntervalMs);
    while (times_of(log, "LISTPV").empty() && now_ms() < poll_wait_until) pump(5);
    CHECK_FALSE(times_of(log, "LISTPV").empty());
    pump(10);  // Let its prompt come back.
    log.clear();
    Tracking track{{static_cast<uint8_t>(API::HardwareOperationID::TrackingJog), 0}, {1, 0, 20}};
    CHECK(send(API::CommandID::ExecuteHardwareOperation, &track, sizeof(track)));
    pump(160);
    API::HardwareOperation stop{static_cast<uint8_t>(API::HardwareOperationID::JointJogStop), 0};
    CHECK(send(API::CommandID::ExecuteHardwareOperation, &stop, sizeof(stop)));
    pump(TelemetrySchedule::kQuietAfterJogMs + kSlackMs);
    toggles = times_of(log, "~");
    CHECK_EQUAL(2, toggles.size());
    std::vector<long> after_track;
    for (long t : times_of(log, "LISTPV"))
      if (t >= toggles[1]) after_track.push_back(t);
    CHECK_EQUAL(1, after_track.size());
    long held = after_track[0] - toggles[1];
    CHECK_TEXT(held >= static_cast<long>(TelemetrySchedule::kQuietAfterJogMs) - 5,
               std::to_string(held).c_str());
    CHECK_TEXT(held <= static_cast<long>(TelemetrySchedule::kQuietAfterJogMs) + kSlackMs,
               std::to_string(held).c_str());
  }
  subscriber.stop();
  close(master);
}
