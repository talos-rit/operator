/**
 * Generic, Hardware Agnostic Arm Interface
*/

#pragma once

#include <stdint.h>
#include <pthread.h>

#include "sub/sub.h"
#include "api/api.h"

class Arm
{
    pthread_t pid;
    bool thread_enable;

private:
    virtual int HandShake() = 0;
    virtual int PolarPan(API_Data_Polar_Pan *pan) = 0;
    virtual int Home(API_Data_Home* home) = 0;

public:
    Arm();
    ~Arm();

    // Thread control
    int Start();
    int Stop();

    bool GetThreadEnable();
    int ProcessBuffer(SUB_Buffer *buf);

    virtual void Poll() = 0;
};

#ifdef __cplusplus
extern "C" {
#endif



#ifdef __cplusplus
}
#endif