/**
 * Generic, Hardware Agnostic Arm Interface
*/

#pragma once

#include <stdint.h>
#include <pthread.h>

#include "sub/sub.h"
#include "api/api.h"

#define ARM_POLL_PERIOD_MS (25)

class Arm
{
private:
    pthread_t pid;
    bool thread_enable;
    Subscriber* sub;

    virtual int HandShake() = 0;
    virtual int PolarPan(API_Data_Polar_Pan *pan) = 0;
    virtual int PolarPanStart(API_Data_Polar_Pan_Start *pan) = 0;
    virtual int PolarPanStop() = 0;
    virtual int Home(API_Data_Home* home) = 0;

public:
    Arm();
    virtual ~Arm();

    // Thread control
    int Start();
    int Stop();

    bool GetThreadEnable();

    /**
     * @brief If there's a buffer in the command queue, it will be processed
     * @returns 0 a buffer has processed properly, 1 if there is no buffer, -1 on failure
    */
    int ProcessQueue();
    int RegisterSubscriber(Subscriber* sub);

    /**
     * @brief Function that is called by the parent Arm thread in a timed loop;
     *        Used miscellaneous tasks that need to be regularly executed.
     * @details Primary purpose is to log data received from Scorbot.
     * Flushes buffer after receiving a line terminator, or after a fixed length of time
    */
    virtual void Poll() = 0;
};