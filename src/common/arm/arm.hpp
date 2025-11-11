/**
 * Generic, Hardware Agnostic Arm Interface
 */

#pragma once

#include <atomic>
#include <thread>

#include "api/api.hpp"
#include "sub/sub.hpp"

constexpr std::chrono::milliseconds ARM_LOOP_PERIOD{10};

class Arm {
 public:
  enum OversteerConfig {
    None,
    Ignore,
    Abort,
  };

  Arm();
  virtual ~Arm();

  // Thread control
  bool start();
  bool stop();

  bool isRunning() const noexcept { return running_.load(); }

  /**
   * @brief If there's a buffer in the command queue, it will be processed
   * @returns true if a buffer has processed properly, false otherwise
   */
  bool processCommand();
  bool registerSubscriber(Subscriber *sub);

  /**
   * @brief Function that is called by the parent Arm thread in a timed loop;
   *        Used miscellaneous tasks that need to be regularly executed.
   * @details Primary purpose is to log data received from Scorbot.
   * Flushes buffer after receiving a line terminator, or after a fixed length
   * of time
   */
  virtual void poll() = 0;

 private:
  std::thread thread_;
  std::atomic<bool> running_{false};
  Subscriber *sub_{nullptr};

  void runLoop();

  virtual int handShake() = 0;
  virtual int polarPan(API::PolarPan *pan) = 0;
  virtual int polarPanStart(API::PolarPanStart *pan) = 0;
  virtual int polarPanStop() = 0;
  virtual int home(API::Home *home) = 0;
};
