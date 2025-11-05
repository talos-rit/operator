#pragma once

#include "api/api.hpp"
#include "arm/arm.hpp"
#include "data/s_list.h"

#define ERV_RX_TIMEOUT_MS 500
#define ERV_CONT_POLAR_PAN_TIMEOUT_MS 500

class Scorbot : public Arm {
 public:
  Scorbot(const char *dev);
  ~Scorbot();

 private:
  char dev[32];
  int fd;
  char polar_pan_cont;
  bool manual_mode;
  OversteerConfig oversteer;
  S_List cmd_buffer;
  struct timeval last_start;

  int handShake();
  int polarPan(API::Requests::PolarPan *pan);
  int polarPanStart(API::Requests::PolarPanStart *pan);
  int polarPanStop();
  int home(API::Requests::Home *home);
  int writeCommandQueue(S_List *cmd_list);
  void poll();
};
