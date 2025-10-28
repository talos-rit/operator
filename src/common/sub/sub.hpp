/**
 * Subscriber Module
 * Acts as the interface for the rest of the Operator system to send and receive
 * messages. This includes, logs, return messages, etc.
 *
 * This interface is intended to be an agnostic interface to whatever messaging
 * implementation is used to communicate with the director.
 */

#pragma once

#include <cstdint>

#include <array>
#include <deque>
#include <mutex>
#include <vector>


class Inbox;  // Forward declaration

constexpr size_t SUB_MSG_LEN = 255;
constexpr size_t SUB_MSG_COUNT = 64;
constexpr size_t SUB_ADDR_LEN = 32;


// Draw here how this queue works. Comes from socket, pulls from free queue, puts info, then puts into command queue, then arm pulls from command queue, processes, then into free queue again. Diagaam in comments below.
//

// Queue types
enum class Sub_Queue : uint8_t {
  Free,    /** Index of Free Queue */
  Command, /** Index of Command Queue */
  Return,  /** Index of Return Queue */
  Log,     /** Index of Log Queue */
  Count,   /** Always last; Equal to length of enum */
};

struct Sub_Buffer {
  std::array<uint8_t, SUB_MSG_LEN> body{};
  uint8_t len{0};

  void reset() noexcept {
    len = 0;
    body.fill(0);
  }
};

class Subscriber {
 public:
  Subscriber();
  ~Subscriber();

  bool start();
  bool stop();
  void Abort();

  /**
   * @brief Enqueues a SUB_Buffer struct from the head of the specified list
   * @param queue Queue to enqueue onto
   * @param buf Buffer to enqueue
   * @returns true on success, false on failure
   */
  bool enqueueBuffer(Sub_Queue queue, Sub_Buffer* buf);

  /**
   * @brief Dequeues a SUB_Buffer struct from the head of the specified list
   * @param queue Queue to dequeue from
   * @returns SUB_Buffer pointer on success, NULL on failure
   */
  Sub_Buffer* dequeueBuffer(Sub_Queue queue);

 private:
  enum class State {
    Dead,
    Init,
    Run,
  };

  struct Queue {
    std::mutex mtx;
    std::deque<Sub_Buffer*> buffers;
  };

  State state_{State::Dead};
  std::array<Queue, static_cast<size_t>(Sub_Queue::Count)> queues_{};
  std::vector<Sub_Buffer> pool_;

  void prepareBuffers() noexcept;
  void destroyBuffers() noexcept;
};
