#pragma once

#include <netinet/in.h>
#include <unistd.h>

#include <atomic>
#include <chrono>
#include <cstddef>
#include <span>
#include <thread>

#include "sub/inbox.hpp"
#include "sub/sub.hpp"
#include "util/file_descriptor.hpp"

#define SOCKET_BUF_LEN 1024
#define SOCKET_POLL_PERIOD_MS 25
#define SOCKET_PING_FREQ_MS 63
#define SOCKET_TIMEOUT_MS (4 * SOCKET_PING_FREQ_MS)

constexpr std::size_t SOCKET_BUF_SIZE = 1024;
constexpr auto SOCKET_POLL_PERIOD = std::chrono::milliseconds(25);
constexpr auto SOCKET_PING_FREQ = std::chrono::milliseconds(63);
constexpr auto SOCKET_TIMEOUT = 4 * SOCKET_PING_FREQ;

class Socket : public Inbox {
 public:
  Socket();
  ~Socket();

  bool start() override;
  void stop() override;
  void registerSubscriber(Subscriber *sub) override;

 private:
  void poll();
  bool init();
  bool waitForConnection();
  bool sendResponse(std::span<const char> msg);

  struct Props {
    FileDescriptor sockfd;
    FileDescriptor connfd;
    sockaddr_in server{};
    sockaddr_in client{};
    int port{61616};
    Subscriber *sub{nullptr};
    std::atomic<bool> running{false};
  } props_;

  std::thread thread_;
};
