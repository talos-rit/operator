/**
 * Generic, Hardware Agnostic Arm Interface
*/

#pragma once

#include <stdint.h>

#include "sub/sub.h"
#include "api/api.h"

class Arm
{
    pthread_t pid;
    bool thread_enable;

private:

    virtual int HandShake() = 0;
    virtual int PolarPan(API_Data_Polar_Pan *pan) = 0;
    virtual int Home() = 0;

public:
    Arm()   {}  // Init
    ~Arm()  {}  // Destroy

    // Thread control
    virtual int Start()             = 0;
    virtual int Stop()              = 0;

    bool GetThreadEnable();
    int ProcessBuffer(SUB_Buffer *buf);
};

#ifdef __cplusplus
extern "C" {
#endif



#ifdef __cplusplus
}
#endif