#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
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

I2CDev::I2CDev(int fd, uint8_t addr)
{
    this->fd = fd;
    this->addr = addr;
}

I2CDev::~I2CDev()
{
}

int I2CDev::FlushQueue()
{

    S_List* list = NULL;
    while((list = PopQueue()))
    {
        memset(msgs, 0, sizeof(msgs));
        uint8_t msg_iter = 0;
        for(S_List_Node *node = list->head; node != list->tail; node = node->next)
        {
            SerialFrame* frame = DATA_LIST_GET_OBJ(node, SerialFrame, node);
            // 0 is write, 1 is read
            if (frame->tx_buf) msgs[msg_iter++] = {.addr = addr, .flags = 0, .len = frame->len, .buf = (uint8_t*) frame->tx_buf};
            if (frame->rx_buf) msgs[msg_iter++] = {.addr = addr, .flags = I2C_M_RD, .len = frame->len, .buf = frame->rx_buf};

            if (frame->callback) frame->callback();
            frame->state = SerialDevice::PROCESSED;
        }

        struct i2c_rdwr_ioctl_data payload = {.msgs = &msgs[0], .nmsgs = msg_iter };
        int ret = 0;
        ret = ioctl(fd, I2C_SLAVE, addr);
        if (-1 == ret) LOG_WARN ("Failed to set I2C device address");
        else LOG_VERBOSE(2, "Set I2C device address: %u", addr);
        ioctl(fd, I2C_RDWR, &payload);
    }

    return 0;
}