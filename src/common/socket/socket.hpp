#pragma once

#include <netinet/in.h>
#include <unistd.h>

#include <atomic>
#include <chrono>
#include <cstddef>
#include <mutex>
#include <span>
#include <string_view>
#include <thread>

#include "arm/arm.hpp"
#include "sub/inbox.hpp"
#include "sub/sub.hpp"
#include "util/file_descriptor.hpp"

constexpr std::size_t SOCKET_BUF_LEN = 1024;
constexpr auto SOCKET_POLL_PERIOD = std::chrono::milliseconds(25);

class Socket : public Inbox, public TelemetrySink {
 public:
  Socket();
  ~Socket();

  bool start() override;
  void stop() override;
  void registerSubscriber(Subscriber *sub) override;
  void sendTelemetry(std::string_view message) override;

 private:
  void poll();
  bool init();
  bool waitForConnection();
  bool sendResponse(std::span<const char> msg);
  std::mutex send_mutex_;

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
