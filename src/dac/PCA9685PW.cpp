#include <string.h>
#include <linux/const.h>

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

    for (uint8_t frame_iter = 0; frame_iter < DAC_PCA_FRAME_LEN; frame_iter++)
    {
        SerialDevice::InitFrame(&frames[frame_iter]);
        DATA_S_List_append(&free, &frames[frame_iter].node);
    }
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

#if 1
int PCA9685PW::prep_queue_transaction(uint8_t addr, uint8_t len, bool read)
{
    const int frame_count = 2;
    if ((addr > 0x45 && addr < 0xFA) || len == 0) STD_FAIL;
    if ((addr < 0x46 && len > 0x45 - addr + 1) ||
        (addr > 0xF9 && len > 0xFF - addr + 1)) STD_FAIL;
    if (DAC_PCA_QUEUE_LEN - queue_iter < 1) STD_FAIL;

    S_List* queue = &queues[queue_iter++];
    SerialDevice::SerialFrame* reg_frames[frame_count];
    memset(&reg_frames, 0, sizeof(reg_frames));
    get_frames(&free, &reg_frames[0], frame_count);

    uint8_t *misc_byte = NULL;
    SerialDevice::SerialFrame* frame = NULL;

    // Configure register
    frame = reg_frames[0];
    frame->len = 1;
    misc_byte = &misc[misc_iter++];     // Claim misc byte (will be released on flush)
    *misc_byte = addr;                  // Set register to Mode 1 register
    frame->tx_buf = misc_byte;          // Assign frame tx_buf to misc byte location
    DATA_S_List_append(queue, &frame->node);

    // Configure destination
    frame = reg_frames[1];
    frame->len = len;

    if (read)   frame->rx_buf = ((uint8_t*) &regs) + addr;
    else        frame->tx_buf = ((uint8_t*) &regs) + addr;

    DATA_S_List_append(queue, &frame->node);

    if(dev.QueueFrames(queue)) STD_FAIL;
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
#endif

int PCA9685PW::InitDevice()
{
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

    staged.MODE[0] = DAC_PCA_FLAG_MODE1_AI;
    QueueWR(0x00, 1);
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