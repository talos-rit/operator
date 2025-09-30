#include "serial/test/dummy_i2c.h"

#include "util/comm.h"
#include "util/array.h"
#include "data/list.h"
#include "data/s_list.h"
#include "log/log.h"

#define LOG_CONSOLE_THRESHOLD_THIS  LOG_THRESHOLD_DEFAULT
#define LOG_FILE_THRESHOLD_THIS     LOG_THRESHOLD_MAX

DummyI2C::DummyI2C(int fd, uint8_t addr) : I2CDev(fd, addr)
{
}

DummyI2C::~DummyI2C()
{
}

int DummyI2C::Echo(uint8_t reg, const uint8_t* msg, uint8_t len)
{
    uint8_t tx;
    S_List commands;
    SerialFrame frames[2];

    DATA_S_List_init(&commands);
    for (uint8_t iter = 0; iter < UTIL_len(frames); iter++)
        SerialDevice::InitFrame(&frames[iter]);

    tx = reg;
    frames[0].tx_buf = &tx;
    frames[0].rx_buf = NULL;
    frames[0].len = 1;

    frames[1].tx_buf = msg;
    frames[1].rx_buf = NULL;
    frames[1].len = len;

    DATA_S_List_append(&commands, &frames[0].node);
    DATA_S_List_append(&commands, &frames[1].node);

    QueueFrames(&commands);
    FlushQueue();
    return 0;
}

int DummyI2C::ReadReg(uint8_t reg, uint8_t* msg, uint8_t len)
{
    uint8_t config[2], tx;
    S_List commands;
    SerialFrame frames[3];

    DATA_S_List_init(&commands);
    for (uint8_t iter = 0; iter < UTIL_len(frames); iter++)
        SerialDevice::InitFrame(&frames[iter]);

    config[0] = 0;
    config[1] = 0b00100001;     // Enable auto increment, disable sleep, otherwise use default values

    frames[0].tx_buf = &config[0];
    frames[0].rx_buf = NULL;
    frames[0].len = 2;

    tx = reg;
    frames[1].tx_buf = &tx;
    frames[1].rx_buf = NULL;
    frames[1].len = 1;

    frames[2].tx_buf = NULL;
    frames[2].rx_buf = msg;
    frames[2].len = len;

    DATA_S_List_append(&commands, &frames[0].node);
    DATA_S_List_append(&commands, &frames[1].node);
    DATA_S_List_append(&commands, &frames[2].node);

    QueueFrames(&commands);
    FlushQueue();
    return 0;
}

