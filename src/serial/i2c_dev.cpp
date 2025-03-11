#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <linux/i2c-dev.h>
#include <linux/i2c.h>
#include <sys/ioctl.h>

#include "serial/i2c_dev.h"

#include "util/comm.h"
#include "data/list.h"
#include "data/s_list.h"
#include "log/log.h"

#define LOG_CONSOLE_THRESHOLD_THIS  LOG_THRESHOLD_DEFAULT
#define LOG_FILE_THRESHOLD_THIS     LOG_THRESHOLD_MAX

#define I2C_M_WR 0

I2CDev::I2CDev(int fd, uint8_t addr)
{
    this->fd = fd;
    this->addr = addr;
}

I2CDev::~I2CDev()
{
}


int I2CDev::OverrideAddr(uint8_t addr)
{
    int ret = ioctl(fd, I2C_SLAVE, addr);
    if (ret == -1) STD_FAIL;
    this->addr = addr;
    return 0;
}

void mark_frames(S_List* list, SerialDevice::SerialState state)
{
    for(S_List_Node *node = list->head; node; node = node->next)
    {
        SerialDevice::SerialFrame* frame = DATA_LIST_GET_OBJ(node, SerialDevice::SerialFrame, node);
        if (frame->callback) frame->callback();
        frame->state = state;
    }
}

int I2CDev::FlushQueue(S_List* list)
{
    if (!list || list->len == 0) STD_FAIL;

    int ret = 0;
    ret = ioctl(fd, I2C_SLAVE, addr);
    if (-1 == ret)
    {
        LOG_WARN ("Failed to set I2C device address: (%d) %s", errno, strerror(errno));
        mark_frames(list, SerialState::FAILED);
        STD_FAIL;
    }
    else LOG_VERBOSE(4, "Set I2C device address: %u", addr);

    memset(msgs, 0, sizeof(msgs));
    uint8_t msg_iter = 0;
    for(S_List_Node *node = list->head; node; node = node->next)
    {
        SerialFrame* frame = DATA_LIST_GET_OBJ(node, SerialFrame, node);
        if (frame->tx_buf) msgs[msg_iter++] = {.addr = addr, .flags = I2C_M_WR, .len = frame->len, .buf = (uint8_t*) frame->tx_buf};
        if (frame->rx_buf) msgs[msg_iter++] = {.addr = addr, .flags = I2C_M_RD, .len = frame->len, .buf = frame->rx_buf};
    }

    struct i2c_rdwr_ioctl_data payload = {.msgs = &msgs[0], .nmsgs = msg_iter };
    ret = ioctl(fd, I2C_RDWR, &payload);
    if (-1 == ret)
    {
        LOG_WARN ("Failed to complete I2C transaction: (%d) %s", errno, strerror(errno));
        mark_frames(list, SerialState::FAILED);
        STD_FAIL;
    }

    mark_frames(list, SerialState::PROCESSED);

    return 0;
}

int I2CDev::FlushQueue()
{

    S_List* list = NULL;
    while((list = PopQueue()))
    {
        if(FlushQueue(list)) STD_FAIL;
    }

    return 0;
}
