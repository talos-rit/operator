#include "socket/socket.hpp"

#include <endian.h>
#include <sys/socket.h>

#include <cerrno>
#include <cstring>
#include <thread>

#include "api/api.hpp"
#include "log/log.hpp"
#include "sub/sub.hpp"

#define LOG_CONSOLE_THRESHOLD_THIS LOG_THRESHOLD_DEFAULT
#define LOG_FILE_THRESHOLD_THIS LOG_THRESHOLD_MAX

Socket::Socket() {
  if (!init()) {
    LOG_ERROR("Socket initialization failed.");
  }
}

Socket::~Socket() { stop(); }

bool Socket::init() {
  int fd = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
  LOG_INFO("Socket fd=%d: Initializing...", fd);
  props_.sockfd = FileDescriptor(fd);

  if (!props_.sockfd.valid()) {
    LOG_ERROR("Could not open socket: (%d) %s", errno, strerror(errno));
  }

  int optval = 1;
  ::setsockopt(props_.sockfd.get(), SOL_SOCKET, SO_REUSEADDR, &optval,
               sizeof(optval));

  props_.server = {};
  props_.server.sin_family = AF_INET;
  props_.server.sin_addr.s_addr = INADDR_ANY;
  props_.server.sin_port = ::htons(props_.port);

  if (::bind(props_.sockfd, reinterpret_cast<struct sockaddr*>(&props_.server),
             sizeof(props_.server)) < 0) {
    LOG_ERROR("Could not bind socket: (%d) %s", errno, strerror(errno));
    return false;
  }

  return true;
};

bool Socket::waitForConnection() {
  LOG_INFO("Waiting for client...");
  socklen_t client_len = sizeof(props_.client);
  while (props_.running && !props_.connfd.valid()) {
    int fd = ::accept(props_.sockfd,
                      reinterpret_cast<struct sockaddr*>(&props_.client),
                      &client_len);
    if (fd > 0) {
      props_.connfd = FileDescriptor(fd);
      LOG_INFO("Client connected.");
      return true;
    }
    if (errno != EAGAIN) {
      LOG_ERROR("Socket accept failed: (%d) %s", errno, strerror(errno));
      return false;
    }

    std::this_thread::sleep_for(SOCKET_POLL_PERIOD);
  }

  LOG_INFO("Socket stopped before connection.");
  return false;
}

bool Socket::sendResponse(std::span<const char> msg) {
  int sent = ::send(props_.connfd, msg.data(), msg.size(), 0);
  if (sent < 0) {
    LOG_ERROR("Failed to send response: (%d) %s", errno, strerror(errno));
    return false;
  }
  LOG_INFO("Response sent (%d bytes)", sent);
  return true;
};

void Socket::poll() {
  std::array<char, SOCKET_BUF_LEN> buffer;
  std::size_t buf_iter = 0;

  if (!waitForConnection()) return;

  while (props_.running) {
    auto remaining = std::span(buffer).subspan(buf_iter);
    int ret =
        ::recv(props_.connfd, remaining.data(), remaining.size(), MSG_DONTWAIT);
    if (ret == -1) {
      if (errno == EAGAIN) {
        std::this_thread::sleep_for(SOCKET_POLL_PERIOD);
        continue;
      }
      LOG_ERROR("Socket recv failed: (%d) %s", errno, strerror(errno));
      break;
    }

    if (ret == 0) {
      LOG_INFO("Client disconnected.");
      props_.connfd.reset();
      if (!waitForConnection()) {
        LOG_ERROR("Reconnection failed.");
        break;
      }
      continue;
    }

    buf_iter += ret;

    while (buf_iter >= sizeof(API::DataHeader) + 2) {
      auto* msg = reinterpret_cast<API::DataWrapper*>(buffer.data());
      uint16_t total_len =
          sizeof(API::DataHeader) + be16toh(msg->header.len) + 2;
      if (buf_iter < total_len) break;

      if (!props_.sub) {
        LOG_WARN("No subscriber registered!");
        break;
      }

      if (auto* buf = props_.sub->dequeueBuffer(Sub_Queue::Free)) {
        buf->len = total_len;
        std::memcpy(&buf->body[0], buffer.data(), total_len);
        props_.sub->enqueueBuffer(Sub_Queue::Command, buf);
        LOG_VERBOSE(2, "Received ICD command");
      } else {
        LOG_WARN("No free buffers available!");
      }

      buf_iter -= total_len;
      std::memmove(buffer.data(), buffer.data() + total_len, buf_iter);

      static const char resp[] = "ACK";
      sendResponse(resp);
    }
  }

  if (props_.connfd.valid()) {
    ::shutdown(props_.connfd, SHUT_RDWR);
    props_.connfd.reset();
  }
}

bool Socket::start() {
  if (!props_.sockfd.valid()) {
    LOG_ERROR("Socket not initialized!");
    return false;
  }

  ::listen(props_.sockfd, 5);
  props_.running = true;
  thread_ = std::thread(&Socket::poll, this);
  LOG_INFO("Socket listening on port %d", props_.port);
  return true;
};

void Socket::stop() {
  props_.running = false;
  if (thread_.joinable()) thread_.join();
  LOG_INFO("Socket stopped.");
}

void Socket::registerSubscriber(Subscriber* sub) { props_.sub = sub; }
