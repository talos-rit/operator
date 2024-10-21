#pragma once

#include "arm/arm.h"

class Scorbot : public Arm
{
    private:
        int HandShake();
        int PolarPan(API_Data_Polar_Pan *pan);
        int Home();

    public:
        Scorbot() {}
        ~Scorbot() {}
};