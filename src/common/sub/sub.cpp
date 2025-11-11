#include "sub/sub.hpp"

#include "log/log.h"

#define LOG_CONSOLE_THRESHOLD_THIS LOG_THRESHOLD_DEFAULT
#define LOG_FILE_THRESHOLD_THIS LOG_THRESHOLD_MAX

void Subscriber::prepareBuffers() noexcept {
  pool_.resize(SUB_MSG_COUNT);
  for (auto &q : queues_) {
    q.buffers.clear();
  }

  auto &freeQueue = queues_[static_cast<size_t>(Sub_Queue::Free)];
  for (auto &buf : pool_) freeQueue.buffers.push_back(&buf);
}

void Subscriber::destroyBuffers() noexcept {
  for (auto &q : queues_) {
    std::lock_guard<std::mutex> lock(q.mtx);
    q.buffers.clear();
  }
  pool_.clear();
}

Subscriber::Subscriber() {
  LOG_INFO("Subscriber Initializing...");

  prepareBuffers();
  state_ = State::Init;
  LOG_INFO("Subscriber Initialized.");
}

Subscriber::~Subscriber() {
  if (state_ != State::Dead) {
    destroyBuffers();
  }
  state_ = State::Dead;
}

bool Subscriber::start() {
  if (state_ != State::Init) {
    LOG_WARN("Subscriber already running or dead.");
    return false;
  }
  state_ = State::Run;
  LOG_INFO("Subscriber started.");
  return true;
}

bool Subscriber::stop() {
  if (state_ != State::Run) {
    LOG_WARN("Subscriber not running.");
    return false;
  }
  state_ = State::Init;
  LOG_INFO("Subscriber stopped.");
  return true;
}

Sub_Buffer *Subscriber::dequeueBuffer(Sub_Queue queue) {
  if (state_ != State::Run) {
    LOG_ERROR("Subscriber DequeueBuffer: Subscriber not running");
    return nullptr;
  }

  const auto queueIndex = static_cast<size_t>(queue);
  if (queueIndex >= queues_.size()) {
    LOG_ERROR("Subscriber DequeueBuffer: Invalid queue index %zu", queueIndex);
    return nullptr;
  }

  auto &q = queues_[queueIndex];
  std::lock_guard<std::mutex> lock(q.mtx);

  if (q.buffers.empty()) {
    return nullptr;
  }

  Sub_Buffer *buf = q.buffers.front();
  q.buffers.pop_front();
  return buf;
}

bool Subscriber::enqueueBuffer(Sub_Queue queue, Sub_Buffer *buf) {
  if (state_ != State::Run) {
    LOG_ERROR("Subscriber EnqueueBuffer: Subscriber not running");
    return false;
  }

  const auto queueIndex = static_cast<size_t>(queue);
  if (queueIndex >= queues_.size()) {
    LOG_ERROR("Subscriber EnqueueBuffer: Invalid queue index %zu", queueIndex);
    return false;
  }

  if (queue == Sub_Queue::Free) {
    buf->reset();
  }

  auto &q = queues_[queueIndex];

  // Block for releasing lock after pushing buffer
  {
    std::lock_guard<std::mutex> lock(q.mtx);
    q.buffers.push_back(buf);
  }

  return true;
}
