#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <linux/const.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>

#include "dac/PCA9685PW.h"

#include "util/comm.h"
#include "log/log.h"

#define LOG_FILE_THRESHOLD_THIS     LOG_THRESHOLD_MAX
#define LOG_CONSOLE_THRESHOLD_THIS  LOG_THRESHOLD_DEFAULT

PCA9685PW::PCA9685PW(int fd, int addr)
{
    dev = I2CDev(fd, addr);
    memset(&regs, 0, sizeof(frames));
    memset(&misc, 0, sizeof(misc));
    memset(&queues[0], 0, sizeof(queues));
    misc_iter = 0;
    queue_iter = 0;
    DATA_S_List_init(&free);
    this->fd = fd;

    for (uint8_t frame_iter = 0; frame_iter < DAC_PCA_FRAME_LEN; frame_iter++)
    {
        SerialDevice::InitFrame(&frames[frame_iter]);
        DATA_S_List_append(&free, &frames[frame_iter].node);
    }

    InitDevice();
}

PCA9685PW::~PCA9685PW()
{
}

static int get_frames(S_List* free, SerialDevice::SerialFrame** reg_frames, uint8_t len)
{
    // Initialize frames
    for (uint8_t i = 0; i < len; i++)
    {
        S_List_Node* node = DATA_S_List_pop(free);

        // If S_List_Pop fails, re-free frames and return early
        if (!node)
        {
            for (uint8_t frame_iter = 0; frame_iter < len; frame_iter++)
            {
                if (!reg_frames[frame_iter]) continue;
                SerialDevice::InitFrame(reg_frames[frame_iter]);
                DATA_S_List_append(free, &reg_frames[frame_iter]->node);
                reg_frames[frame_iter] = NULL;
            }

            STD_FAIL;
        }

        reg_frames[i] = DATA_LIST_GET_OBJ(node, SerialDevice::SerialFrame, node);
    }

    return 0;
}

/**
 * @returns NULL on failure, Queue pointer on success
 */
int PCA9685PW::prep_queue_transaction(uint8_t addr, uint8_t len, bool read)
{
    if ((addr > 0x45 && addr < 0xFA) || len == 0) STD_FAIL;
    if ((addr < 0x46 && len > 0x45 - addr + 1) ||
        (addr > 0xF9 && len > 0xFF - addr + 1)) STD_FAIL;
    if (DAC_PCA_QUEUE_LEN - queue_iter < 1) STD_FAIL;

    S_List* queue = &queues[queue_iter++];
    SerialDevice::SerialFrame* reg_frames[2];
    memset(&reg_frames, 0, sizeof(reg_frames));
    get_frames(&free, &reg_frames[0], read ? 2 : 1);

    uint8_t *misc_byte = NULL;
    SerialDevice::SerialFrame* frame = NULL;

    // Configure register
    frame = reg_frames[0];
    frame->len = read ? 1 : 1 + len;
    misc_byte = &misc[misc_iter];       // Claim misc byte (will be released on flush)
    misc_iter += frame->len;            // Adjust misc_iter
    misc_byte[0] = addr;                // Set register to Mode 1 register
    frame->tx_buf = misc_byte;          // Assign frame tx_buf to misc byte location
    DATA_S_List_append(queue, &frame->node);

    if (read)
    {
        frame = reg_frames[1];
        frame->rx_buf = ((uint8_t*) &regs) + addr;
        frame->len = len;
        DATA_S_List_append(queue, &frame->node);
    }
    else
    {
        memcpy(&misc_byte[1],((uint8_t*)&staged) + addr, len);
    }

    return 0;
}

int PCA9685PW::QueueRD(uint8_t addr, uint8_t len)
{
    return prep_queue_transaction(addr, len, true);
}

int PCA9685PW::QueueWR(uint8_t addr, uint8_t len)
{
    return prep_queue_transaction(addr, len, false);
}

int PCA9685PW::ResetDevice()
{
    // Reset has to be special and use a completely unique device address;
    // No easy way to circuvent it; way easier to just manually reimplement this

    const int rst_addr = 0x00;
    int ret = 0;
    ret = ioctl(fd, I2C_SLAVE, rst_addr);
    if (-1 == ret)
    {
        LOG_WARN ("Failed to set I2C device address: (%d) %s", errno, strerror(errno));
        STD_FAIL;
    }
    else LOG_VERBOSE(4, "Set I2C device address: %u", rst_addr);

    uint8_t buf = 0x06;
    struct i2c_msg msg = {.addr = rst_addr, .flags = 0, .len = 1, .buf = &buf};
    struct i2c_rdwr_ioctl_data payload = {.msgs = &msg, .nmsgs = 1 };
    ret = ioctl(fd, I2C_RDWR, &payload);
    if (-1 == ret)
    {
        LOG_WARN ("Failed to complete I2C transaction: (%d) %s", errno, strerror(errno));
        STD_FAIL;
    }

    return 0;
}

int PCA9685PW::InitDevice()
{
    // Declare reg variables
    SerialDevice::SerialFrame* reg_frames[1];
    if(get_frames(&free, &reg_frames[0], 1)) STD_FAIL;

    // Reset device
    ResetDevice();
    usleep(500);

    // Configure mode registers
    uint8_t buf[3] = {0, DAC_PCA_FLAG_MODE1_AI, 0};
    S_List* queue = &queues[queue_iter++];
    reg_frames[0]->len = 3;
    reg_frames[0]->tx_buf = &buf[0];
    DATA_S_List_append(queue, &reg_frames[0]->node);

    UpdateRegisters();
    FlushQueues();
    return 0;
}

int PCA9685PW::UpdateRegisters()
{
    const int len = 5;
    if (free.len < len) STD_FAIL;
    if (DAC_PCA_MISC_LEN - misc_iter < 3) STD_FAIL;

    // Declare frames
    SerialDevice::SerialFrame* reg_frames[len];
    memset(&reg_frames, 0, sizeof(reg_frames));
    int ret = get_frames(&free, &reg_frames[0], len);
    if (-1 == ret) STD_FAIL;

    QueueRD(0x00, 0x45);
    QueueRD(0xFA, 0x06);

    return 0;
}

int PCA9685PW::SetDutyCycle(uint8_t channel, uint16_t value)
{
    if (channel > 15) STD_FAIL;
    if (value > 4095) STD_FAIL;
    Channel* chan = &staged.LEDs[channel];
    memset(chan, 0, sizeof(Channel));

    if (0 == value)         chan->OFF_H |= _BITULL(4);
    else if (4095 == value) chan->ON_H  |= _BITULL(4);
    else
    {
        // No use for delay; set to 0 (leave LED_ON at 0)
        chan->OFF_L = value & 0xFF;
        chan->OFF_H = (value >> 8) & 0x0F;
    }

    QueueWR((uint8_t)(((uint8_t*) chan) - ((uint8_t*) &staged)), sizeof(Channel));
    return 0;
}

int PCA9685PW::GetDutyCycle(uint8_t channel)
{
    if (!inited) STD_FAIL;
    if (channel > 15) STD_FAIL;

    // All off / All on
    Channel* chan = &regs.LEDs[channel];
    if (chan->OFF_H & _BITULL(4)) return 0;
    if (chan->ON_H  & _BITULL(4)) return 4096;

    uint16_t off = (chan->OFF_H << 8) | chan->OFF_L;
    uint16_t on  = (chan->ON_H  << 8) | chan->ON_L;
    return (off > on) ? (off - on) : (4095 - on) + off;
}

int PCA9685PW::FlushQueues()
{
    // Flush to bus
    for (uint8_t iter = 0; iter < queue_iter; iter++)
    {
        if (queues[iter].len == 0) continue;
        dev.FlushQueue(&queues[iter]);
    }

    memset(&queues, 0, sizeof(queues));
    memset(&misc, 0, sizeof(misc));
    queue_iter = 0;
    misc_iter = 0;
    return 0;
}