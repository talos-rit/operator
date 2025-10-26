#pragma once

#include <netinet/in.h>
#include <unistd.h>

#include <atomic>
#include <chrono>
#include <cstddef>
#include <span>
#include <thread>

#include "sub/sub.hpp"
#include "sub/inbox.hpp"

#define SOCKET_BUF_LEN 1024
#define SOCKET_POLL_PERIOD_MS 25
#define SOCKET_PING_FREQ_MS 63
#define SOCKET_TIMEOUT_MS (4 * SOCKET_PING_FREQ_MS)

constexpr std::size_t SOCKET_BUF_SIZE = 1024;
constexpr auto SOCKET_POLL_PERIOD = std::chrono::milliseconds(25);
constexpr auto SOCKET_PING_FREQ = std::chrono::milliseconds(63);
constexpr auto SOCKET_TIMEOUT = 4 * SOCKET_PING_FREQ;

class FileDescriptor {
 protected:
  int fd_{-1};

 public:
  FileDescriptor() = default;
  explicit FileDescriptor(int fd) : fd_(fd) {}
  ~FileDescriptor() { reset(); }

  FileDescriptor(const FileDescriptor &) = delete;
  FileDescriptor &operator=(const FileDescriptor &) = delete;

  FileDescriptor(FileDescriptor &&other) noexcept : fd_(other.fd_) {
    other.fd_ = -1;
  }

  FileDescriptor &operator=(FileDescriptor &&other) noexcept {
    if (this != &other) {
      reset();
      fd_ = other.fd_;
      other.fd_ = -1;
    }
    return *this;
  }

  int get() noexcept { return fd_; }
  bool valid() const noexcept { return fd_ != -1; }
  void reset() noexcept {
    if (fd_ != -1) {
      ::close(fd_);
      fd_ = -1;
    }
  }
  operator int() const noexcept { return fd_; }
};

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
