#pragma once

#include "sub/sub.h"

class Subscriber;

/**
 * @class Subscriber Messenger Interface
 * @details Serves to separate the rest of the Talos Operator system from the
 * messenger implementation (AMQ, HTTP, etc...)
 */
class Inbox {
 protected:
  Subscriber *sub;
  void OnMessage();

 public:
  /**
   * @brief Constructor
   */
  Inbox() {}

  /**
   * @brief Destructor
   */
  virtual ~Inbox() {}

  /**
   * @brief Starts the Messenger service
   * @return true on success, false on failure
   */
  virtual bool start() = 0;

  /**
   * @brief Stops the Messenger service
   * @return true on success, false on failure
   */
  virtual void stop() = 0;

  virtual void registerSubscriber(Subscriber *sub) = 0;
};
