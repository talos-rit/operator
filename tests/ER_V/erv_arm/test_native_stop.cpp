#include <CppUTest/TestHarness.h>
#include <endian.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <cstdarg>
#include <cstdlib>
#include <cstring>
#include <string>
#include "erv_arm/erv.hpp"

extern "C" int __real_ioctl(int, unsigned long, ...);
extern "C" int __wrap_ioctl(int fd, unsigned long request, ...) {
  va_list args;
  va_start(args, request);
  void* arg = va_arg(args, void*);
  va_end(args);
  const char* name = ttyname(fd);
  if (name && strncmp(name, "/dev/pts/", 9) == 0) {
    if (request == TIOCMGET) { *static_cast<int*>(arg) = TIOCM_DTR | TIOCM_RTS; return 0; }
    if (request == TIOCMSET) return 0;
  }
  return __real_ioctl(fd, request, arg);
}

static size_t occurrences(const std::string& text, const char* token) {
  size_t n = 0, position = 0;
  while ((position = text.find(token, position)) != std::string::npos) {
    ++n; position += strlen(token);
  }
  return n;
}

TEST_GROUP(NativeStop) {};

TEST(NativeStop, RepeatedIdleStopsDoNotQueueReferenceWorkAndWatchdogStillStops) {
  int master = posix_openpt(O_RDWR | O_NOCTTY | O_NONBLOCK);
  CHECK(master >= 0);
  CHECK_EQUAL(0, grantpt(master));
  CHECK_EQUAL(0, unlockpt(master));
  Subscriber subscriber;
  CHECK(subscriber.start());
  {
    Scorbot robot(ptsname(master));
    robot.registerSubscriber(&subscriber);
    auto pump = [&](int milliseconds) {
      std::string output;
      for (int t = 0; t < milliseconds; t += 10) {
        static_cast<Arm&>(robot).poll();
        char bytes[1024];
        ssize_t n;
        while ((n = read(master, bytes, sizeof(bytes))) > 0) {
          output.append(bytes, n);
          // Model ACL's prompt after commands, manual-mode transitions, and
          // interruption of telemetry. No real serial device is opened.
          if (memchr(bytes, '\r', n) || memchr(bytes, '~', n) || memchr(bytes, '\003', n))
            CHECK_EQUAL(1, write(master, ">", 1));
        }
        usleep(10000);
      }
      return output;
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
    pump(1200); // Drain the simulated startup HOME queue.
    for (int n = 0; n < 10; ++n) CHECK(send(API::CommandID::PolarPanStop, nullptr, 0));
    std::string idle = pump(1400);
    CHECK_EQUAL(0, occurrences(idle, "DEFP DELTA"));
    CHECK_EQUAL(0, occurrences(idle, "HERE DELTA"));

    struct __attribute__((packed)) Joint {
      API::HardwareOperation operation;
      API::JointJogStart jog;
    } joint{{static_cast<uint8_t>(API::HardwareOperationID::JointJogStart), 0}, {2, 1}};
    CHECK(send(API::CommandID::ExecuteHardwareOperation, &joint, sizeof(joint)));
    std::string first_jog = pump(100);
    CHECK(first_jog.find('w') != std::string::npos);
    for (int n = 0; n < 10; ++n) CHECK(send(API::CommandID::PolarPanStop, nullptr, 0));
    std::string stopped = pump(100);
    CHECK(stopped.find('~') != std::string::npos);
    CHECK_EQUAL(0, occurrences(stopped, "DEFP DELTA"));
    CHECK_EQUAL(0, occurrences(stopped, "HERE DELTA"));

    CHECK(send(API::CommandID::ExecuteHardwareOperation, &joint, sizeof(joint)));
    CHECK(pump(100).find('w') != std::string::npos);
    std::string expired = pump(650);
    CHECK(expired.find('~') != std::string::npos);
    CHECK_EQUAL(0, occurrences(expired, "HERE DELTA"));
    CHECK(pump(100).find('w') == std::string::npos);

    API::PolarPan position{static_cast<int32_t>(htobe32(10)), 0, 0, htobe32(500)};
    CHECK(send(API::CommandID::PolarPan, &position, sizeof(position)));
    std::string positioned = pump(2600);
    CHECK_EQUAL(1, occurrences(positioned, "DEFP DELTA"));
    CHECK_EQUAL(1, occurrences(positioned, "HERE DELTA"));
    CHECK(positioned.find("HERE DELTA") < positioned.find("SHIFT"));
    CHECK(positioned.find("SHIFT") != std::string::npos);
  }
  subscriber.stop();
  close(master);
}
