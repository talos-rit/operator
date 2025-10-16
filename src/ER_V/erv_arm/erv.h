#pragma once

#include "acl/acl.h"
#include "arm/arm.h"

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

  int HandShake();
  int PolarPan(API_Data_Polar_Pan *pan);
  int PolarPanStart(API_Data_Polar_Pan_Start *pan);
  int PolarPanStop();
  int Home(API_Data_Home *home);
  int WriteCommandQueue(S_List *cmd_list);
  void Poll();
};