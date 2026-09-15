#pragma once

#include "api/api.hpp"
#include "arm/arm.hpp"
#include "data/s_list.h"

#define ERV_RX_TIMEOUT_MS 500
#define ERV_CONT_POLAR_PAN_TIMEOUT_MS 500
#define ERV_HOME_SETTLE_MS 10000
#define ERV_TELEMETRY_INTERVAL_MS 100

class Scorbot : public Arm {
 public:
  Scorbot(const char* dev);
  ~Scorbot();

 private:
  char dev[32];
  int fd;
  char polar_pan_cont;
  bool manual_mode;
  bool direct_mode;
  bool telemetry_request_pending;
  uint16_t telemetry_delay_ms;
  OversteerConfig oversteer;
  S_List cmd_buffer;
  struct timeval last_start;
  struct timeval telemetry_not_before;

  int handShake();
  int polarPan(API::PolarPan* pan);
  int polarPanStart(API::PolarPanStart* pan);
  int polarPanStop();
  int executeHardwareOperation(API::HardwareOperation* operation);
  int home(API::Home* home);
  int writeCommandQueue(S_List* cmd_list);
  void poll();
};
