#pragma once

#include "arm/arm.h"

class Scorbot : public Arm
{
    private:
        char dev[32];
        int fd;

        int HandShake();
        int PolarPan(API_Data_Polar_Pan *pan);
        int Home(API_Data_Home *home);
        void Poll();

    public:
        Scorbot(const char* dev);
        ~Scorbot();
};