#include <CppUTest/TestHarness.h>
#include <arpa/inet.h>
#include <endian.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <thread>
#include <vector>

#include "api/api.hpp"
#include "socket/socket.hpp"
#include "sub/sub.hpp"

namespace {

uint16_t freePort() {
  int fd = ::socket(AF_INET, SOCK_STREAM, 0);
  sockaddr_in address{};
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
  ::bind(fd, reinterpret_cast<sockaddr*>(&address), sizeof(address));
  socklen_t size = sizeof(address);
  ::getsockname(fd, reinterpret_cast<sockaddr*>(&address), &size);
  ::close(fd);
  return ntohs(address.sin_port);
}

}  // namespace

TEST_GROUP(SocketLatency) {};

// A command that arrives while Operator is idle used to wait for the receive
// loop's 25 ms sleep to end (about 12 ms on average) before being read.
TEST(SocketLatency, IdleCommandIsReadWithoutWaitingOutAPollPeriod) {
  Subscriber sub;
  CHECK(sub.start());
  uint16_t port = freePort();
  Socket server(port, htonl(INADDR_LOOPBACK));
  server.registerSubscriber(&sub);
  CHECK(server.start());

  int client = ::socket(AF_INET, SOCK_STREAM, 0);
  timeval timeout{2, 0};
  ::setsockopt(client, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
  sockaddr_in address{};
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
  address.sin_port = htons(port);
  CHECK_EQUAL(0, ::connect(client, reinterpret_cast<sockaddr*>(&address), sizeof(address)));

  std::vector<char> frame(sizeof(API::DataHeader) + 4 + 1, 0);
  API::DataHeader header{};
  header.len = htobe16(4);
  std::memcpy(frame.data(), &header, sizeof(header));

  std::vector<double> latencies_ms;
  for (int i = 0; i < 20; i++) {
    // Let the server go idle first, at a different phase each time.
    std::this_thread::sleep_for(std::chrono::milliseconds(30 + (i * 7) % 25));
    auto sent = std::chrono::steady_clock::now();
    CHECK_EQUAL((ssize_t)frame.size(), ::send(client, frame.data(), frame.size(), 0));
    char reply[8];
    CHECK(::recv(client, reply, sizeof(reply), 0) >= 4);
    latencies_ms.push_back(
        std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - sent).count());
    if (Sub_Buffer* buf = sub.dequeueBuffer(Sub_Queue::Command)) sub.enqueueBuffer(Sub_Queue::Free, buf);
  }
  ::close(client);
  server.stop();
  sub.stop();

  std::sort(latencies_ms.begin(), latencies_ms.end());
  double median = latencies_ms[latencies_ms.size() / 2];
  printf("\nidle command ACK latency: median %.2f ms, max %.2f ms\n", median, latencies_ms.back());
  // Well under the 25 ms poll period, with room for a slow CI runner.
  CHECK(median < 5.0);
}
