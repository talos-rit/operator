#include <CppUTest/TestHarness.h>
#include <arpa/inet.h>
#include <endian.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

#include <cstring>
#include <vector>

#include "api/api.hpp"
#include "socket/socket.hpp"
#include "sub/sub.hpp"

namespace {

uint16_t reservePort() {
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

int connectTo(uint16_t port) {
  int fd = ::socket(AF_INET, SOCK_STREAM, 0);
  timeval timeout{2, 0};
  ::setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
  sockaddr_in address{};
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
  address.sin_port = htons(port);
  CHECK_EQUAL(0, ::connect(fd, reinterpret_cast<sockaddr*>(&address),
                           sizeof(address)));
  return fd;
}

std::vector<char> frame(uint16_t payload_len) {
  std::vector<char> bytes(sizeof(API::DataHeader) + payload_len + 1, 0);
  API::DataHeader header{};
  header.len = htobe16(payload_len);
  std::memcpy(bytes.data(), &header, sizeof(header));
  return bytes;
}

}  // namespace

TEST_GROUP(SocketFraming) {};

TEST(SocketFraming, OversizedFrameDropsClientAndServerKeepsListening) {
  Subscriber sub;
  CHECK(sub.start());
  uint16_t port = reservePort();
  Socket server(port, htonl(INADDR_LOOPBACK));
  server.registerSubscriber(&sub);
  CHECK(server.start());

  // Larger than a Sub_Buffer but small enough to arrive in full.
  int attacker = connectTo(port);
  auto oversized = frame(SUB_MSG_LEN);
  CHECK_EQUAL((ssize_t)oversized.size(),
              ::send(attacker, oversized.data(), oversized.size(), 0));
  char reply[8];
  CHECK_EQUAL(0, ::recv(attacker, reply, sizeof(reply), 0));
  ::close(attacker);
  CHECK(sub.dequeueBuffer(Sub_Queue::Command) == nullptr);

  // A well-formed frame from the next client is still accepted.
  int client = connectTo(port);
  auto valid = frame(4);
  CHECK_EQUAL((ssize_t)valid.size(),
              ::send(client, valid.data(), valid.size(), 0));
  CHECK(::recv(client, reply, sizeof(reply), 0) >= 4);
  CHECK(std::memcmp(reply, "ACK\n", 4) == 0);
  ::close(client);

  Sub_Buffer* buf = sub.dequeueBuffer(Sub_Queue::Command);
  CHECK(buf != nullptr);
  CHECK_EQUAL(valid.size(), buf->len);

  server.stop();
}
