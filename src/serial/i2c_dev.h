#pragma once

#include <stddef.h>
#include <stdint.h>
#include <linux/i2c.h>

#include "serial/serial.h"

#define I2C_MSG_COUNT 64

class I2CDev: public SerialDevice
{
private:
    int fd;
    uint8_t addr;
    i2c_msg msgs[I2C_MSG_COUNT];

public:
    I2CDev(int fd = -1, uint8_t addr = 0);
    ~I2CDev();

    int OverrideAddr(uint8_t addr);

    /**
     * @brief Flushed the queue list of frames to the communication bus
     * @returns 0 on success, -1 on failure
    */
    int FlushQueue();

    /**
     * @brief Flushed a provided list of frames to the communication bus
     * @returns 0 on success, -1 on failure
    */
    int FlushQueue(S_List* queue);
};
