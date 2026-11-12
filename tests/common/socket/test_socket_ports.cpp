#include <CppUTest/TestHarness.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include "socket/socket.hpp"

TEST_GROUP(IndependentOperatorPorts) {};

TEST(IndependentOperatorPorts, TwoListenersUseTheirConfiguredPorts) {
  int reservations[2];
  uint16_t ports[2];
  for (int i = 0; i < 2; ++i) {
    reservations[i] = ::socket(AF_INET, SOCK_STREAM, 0);
    CHECK(reservations[i] >= 0);
    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    CHECK_EQUAL(0, bind(reservations[i], reinterpret_cast<sockaddr*>(&address), sizeof(address)));
    socklen_t size = sizeof(address);
    CHECK_EQUAL(0, getsockname(reservations[i], reinterpret_cast<sockaddr*>(&address), &size));
    ports[i] = ntohs(address.sin_port);
  }
  close(reservations[0]);
  close(reservations[1]);
  Socket first(ports[0], htonl(INADDR_LOOPBACK));
  Socket second(ports[1], htonl(INADDR_LOOPBACK));
  CHECK(first.start());
  CHECK(second.start());
  for (auto port : ports) {
    int client = ::socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    address.sin_port = htons(port);
    CHECK_EQUAL(0, connect(client, reinterpret_cast<sockaddr*>(&address), sizeof(address)));
    close(client);
  }
  first.stop();
  second.stop();
}
