#pragma once

#include "arm/arm.h"

class Ichor : public Arm
{
    public:
        Ichor();
        ~Ichor();

    private:
        char polar_pan_cont;
        bool manual_mode;
        OversteerConfig oversteer;

        int HandShake();
        int PolarPan(API_Data_Polar_Pan *pan);
        int PolarPanStart(API_Data_Polar_Pan_Start *pan);
        int PolarPanStop();
        int Home(API_Data_Home *home);
        void Poll();
};