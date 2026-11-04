#include <CppUTest/TestHarness.h>
#include <endian.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstdlib>
#include <algorithm>
#include <cstring>
#include <string>
#include "erv_arm/erv.hpp"

// A silent PTY models the current ESP bridge: no prompt and no telemetry.
// All commands pass through the real binary API parser and Arm dispatcher.
TEST_GROUP(EspProfile) {};

TEST(EspProfile, SilentSerialSupportsJogStopWatchdogAndHomeWithoutNativeAcl) {
  int master = posix_openpt(O_RDWR | O_NOCTTY | O_NONBLOCK);
  CHECK(master >= 0);
  CHECK_EQUAL(0, grantpt(master));
  CHECK_EQUAL(0, unlockpt(master));
  Subscriber subscriber;
  CHECK(subscriber.start());
  {
    Scorbot robot(ptsname(master), Scorbot::ControllerProfile::EspOneAxis);
    robot.registerSubscriber(&subscriber);
    auto pump = [&](int milliseconds) {
      for (int i = 0; i < milliseconds; i += 10) {
        static_cast<Arm&>(robot).poll();
        usleep(10000);
      }
      std::string output;
      char bytes[512];
      ssize_t n;
      while ((n = read(master, bytes, sizeof(bytes))) > 0) output.append(bytes, n);
      return output;
    };
    unsigned sequence = 0;
    auto send = [&](API::CommandID command, const void* payload, size_t length) {
      auto* buffer = subscriber.dequeueBuffer(Sub_Queue::Free);
      CHECK(buffer != nullptr);
      API::DataHeader header{htobe32(++sequence), 0,
                            htobe16(static_cast<uint16_t>(command)),
                            htobe16(static_cast<uint16_t>(length))};
      memcpy(buffer->body.data(), &header, sizeof(header));
      if (length) memcpy(buffer->body.data() + sizeof(header), payload, length);
      buffer->len = sizeof(header) + length + 1;
      CHECK(subscriber.enqueueBuffer(Sub_Queue::Command, buffer));
      return robot.processCommand();
    };
    std::string startup = pump(300);
    CHECK(startup.find("clrbuf\r") != std::string::npos);
    CHECK(startup.find("HOME") == std::string::npos);
    CHECK(startup.find("LISTPV") == std::string::npos);
    API::PolarPanStart base{1, 0};
    CHECK(send(API::CommandID::PolarPanStart, &base, sizeof(base)));
    std::string first = pump(100);
    CHECK(first.find("~") != std::string::npos);
    CHECK(first.find("q") != std::string::npos);
    CHECK(send(API::CommandID::PolarPanStop, nullptr, 0));
    std::string stopped = pump(200);
    CHECK(stopped.find("~") != std::string::npos);
    CHECK(stopped.find("clrbuf\r") != std::string::npos);
    CHECK(stopped.find("HERE") == std::string::npos);

    base = {-1, 0};
    CHECK(send(API::CommandID::PolarPanStart, &base, sizeof(base)));
    CHECK(pump(100).find("1") != std::string::npos);
    std::string timeout = pump(650);
    CHECK(timeout.find("~") != std::string::npos);
    CHECK(pump(150).find("1") == std::string::npos);

    struct __attribute__((packed)) Tracking {
      API::HardwareOperation operation;
      API::TrackingJog jog;
    } tracking{{static_cast<uint8_t>(API::HardwareOperationID::TrackingJog), 0}, {1, 0, 200}};
    CHECK(send(API::CommandID::ExecuteHardwareOperation, &tracking, sizeof(tracking)));
    std::string repeated = pump(240);
    CHECK(std::count(repeated.begin(), repeated.end(), 'q') >= 4);
    tracking.jog = {0, 1, 10};
    CHECK_FALSE(send(API::CommandID::ExecuteHardwareOperation, &tracking, sizeof(tracking)));
    CHECK(pump(200).find("clrbuf\r") != std::string::npos);
    CHECK(pump(150).find('q') == std::string::npos);

    API::HardwareOperation unsupported{static_cast<uint8_t>(API::HardwareOperationID::EnableControl), 0};
    CHECK_FALSE(send(API::CommandID::ExecuteHardwareOperation, &unsupported, sizeof(unsupported)));
    CHECK(pump(150).find("CON\r") == std::string::npos);
    API::Home home{};
    CHECK(send(API::CommandID::Home, &home, sizeof(home)));
    std::string homing = pump(300);
    CHECK(homing.find("HOME\r") != std::string::npos);
    CHECK(homing.find("DEFP") == std::string::npos);
    CHECK(homing.find("HERE") == std::string::npos);
    CHECK(homing.find("LISTPV") == std::string::npos);
  }
  subscriber.stop();
  close(master);
}
