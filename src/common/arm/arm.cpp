#include "arm/arm.hpp"

#include <unistd.h>

#include "api/api.hpp"
#include "log/log.hpp"
#include "sub/sub.hpp"

#define LOG_CONSOLE_THRESHOLD_THIS LOG_THRESHOLD_MAX
#define LOG_FILE_THRESHOLD_THIS LOG_THRESHOLD_MAX

Arm::Arm() = default;

Arm::~Arm() {}

void Arm::runLoop() {
  while (running_) {
    processCommand();
    poll();
    std::this_thread::sleep_for(ARM_LOOP_PERIOD);
    continue;
  }
}

bool Arm::start() {
  running_.store(true);
  thread_ = std::thread(&Arm::runLoop, this);
  return true;
}

bool Arm::stop() {
  running_.store(false);
  if (thread_.joinable()) thread_.join();
  return true;
}

bool Arm::registerSubscriber(Subscriber* sub) {
  if (!sub) return false;
  sub_ = sub;
  return true;
}

bool Arm::processCommand() {
  Sub_Buffer* buf = sub_->dequeueBuffer(Sub_Queue::Command);

  if (!buf) {
    return false;
  }
  if (API::validate_command(&buf->body[0], buf->len)) {
    sub_->enqueueBuffer(Sub_Queue::Free, buf);
    return false;
  };
  API::DataWrapper* cmd = (API::DataWrapper*)&buf->body;

  int status = 0;
  switch (static_cast<API::CommandID>(cmd->header.cmd_id)) {
    case API::CommandID::Handshake:
      LOG_INFO("Handshake Recieved");
      if (handShake()) status = -1;
      break;
    case API::CommandID::PolarPan:
      LOG_INFO("Polar Pan Recieved");
      if (polarPan((API::PolarPan*)&cmd->payload_head)) status = -1;
      break;
    case API::CommandID::Home:
      LOG_INFO("Home Received");
      if (home((API::Home*)&cmd->payload_head)) status = -1;
      break;
    case API::CommandID::PolarPanStart:
      LOG_INFO("Polar Pan Start Received");
      if (polarPanStart((API::PolarPanStart*)&cmd->payload_head)) status = -1;
      break;
    case API::CommandID::PolarPanStop:
      LOG_INFO("Polar Pan Stop Received");
      if (polarPanStop()) status = -1;
      break;
    default:
      LOG_IEC();
      status = -1;
      break;
  }

  sub_->enqueueBuffer(Sub_Queue::Free, buf);

  return status == 0;
}
