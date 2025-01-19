#pragma once

#include "arm/arm.h"
#include "acl/acl.h"

#define ERV_RX_TIMEOUT_MS 500

class Scorbot : public Arm
{
    public:

        Scorbot(const char* dev);
        ~Scorbot();

    private:
        char dev[32];
        int fd;
        char polar_pan_cont;
        bool manual_mode;
        OversteerConfig oversteer;
        S_List cmd_buffer;


        int HandShake();
        int PolarPan(API_Data_Polar_Pan *pan);
        int PolarPanStart(API_Data_Polar_Pan_Start *pan);
        int PolarPanStop();
        int Home(API_Data_Home *home);
        int WriteCommandQueue(S_List *cmd_list);
        void Poll();
};