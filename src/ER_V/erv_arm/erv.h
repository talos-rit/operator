#pragma once

#include "arm/arm.h"
#include "acl/acl.h"

#define ERV_RX_TIMEOUT_MS 500

class Scorbot : public Arm
{
    private:
        char dev[32];
        int fd;

        int HandShake();
        int PolarPan(API_Data_Polar_Pan *pan);
        int PolarPanStart(API_Data_Polar_Pan_Start *pan);
        int PolarPanStop();
        int Home(API_Data_Home *home);
        int WriteCommandQueue(S_List cmd_list);
        void Poll();

    public:
        Scorbot(const char* dev);
        ~Scorbot();
};