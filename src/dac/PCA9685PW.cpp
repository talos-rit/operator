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
    DATA_S_List_init(&queue);
    DATA_S_List_init(&free);
    memset(&misc, 0, sizeof(misc));
    misc_iter = 0;

    for (uint8_t frame_iter = 0; frame_iter < DAC_PCA_FRAME_LEN; frame_iter++)
    {
        SerialDevice::InitFrame(&frames[frame_iter]);
        DATA_S_List_append(&free, &frames[frame_iter].node);
    }
}

PCA9685PW::~PCA9685PW()
{
}

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

    // Initialize frames
    for (uint8_t i = 0; i < len; i++)
    {
        S_List_Node* node = DATA_S_List_pop(&free);

        // If S_List_Pop fails, re-free frames and return early
        if (!node)
        {
            for (uint8_t frame_iter = 0; frame_iter < len; frame_iter++)
            {
                if (!reg_frames[frame_iter]) continue;
                SerialDevice::InitFrame(reg_frames[frame_iter]);
                DATA_S_List_append(&free, &reg_frames[frame_iter]->node);
                reg_frames[frame_iter] = NULL;
            }

            STD_FAIL;
        }

        reg_frames[i] = DATA_LIST_GET_OBJ(node, SerialDevice::SerialFrame, node);
    }

    // Populate frames
    uint8_t *misc_byte = NULL;
    SerialDevice::SerialFrame *frame = NULL;

    // Setup init register (TODO: move to InitDevice)
    frame = reg_frames[0];                      // Could use pointer arithmatic; too easy to mess up
    frame->len = 2;
    misc_byte = &misc[misc_iter];               // Claim misc byte (will be released on flush)
    misc_iter += frame->len;                    // Increment iterator (prevents byte from being used elsewhere)
    misc_byte[0] = 0x00;                        // Set register to Mode 1 register
    misc_byte[1] = DAC_PCA_FLAG_MODE1_AI;       // Enable Auto Increment in Mode 1 register
    frame->tx_buf = misc_byte;                  // Assign frame tx_buf to misc byte location

    // Setup reading first 70 registers
    frame = reg_frames[1];                      // Could use pointer arithmatic; too easy to mess up
    frame->len = 1;
    misc_byte = &misc[misc_iter];               // Claim misc byte (will be released on flush)
    misc_iter += frame->len;                    // Increment iterator (prevents byte from being used elsewhere)
    misc_byte[0] = 0x00;                        // Set register to Mode 1 register
    frame->tx_buf = misc_byte;                  // Assign frame tx_buf to misc byte location

    // Assign destination for first 70 registers
    frame = reg_frames[2];
    frame->rx_buf = &regs.MODE[0];
    frame->len = offsetof(PCA_Regs, ALL_LED);   // Equals length of primary registers.

    #if 1
    // Setup reading last 6 registers
    frame = reg_frames[3];
    frame->len = 1;
    misc_byte = &misc[misc_iter];               // Claim misc byte (will be released on flush)
    misc_iter += frame->len;                    // Increment iterator (prevents byte from being used elsewhere)
    misc_byte[0] = 0xFA;                        // Set register to Mode 1 register
    frame->tx_buf = misc_byte;                  // Assign frame tx_buf to misc byte location

    // Assign destination for last 6 registers
    frame = reg_frames[4];
    frame->rx_buf = (uint8_t*) &regs.ALL_LED;
    frame->len = sizeof(PCA_Regs) - reg_frames[1]->len;   // Rest of the struct
    #endif

    // Enqueue frames
    for (uint8_t i = 0; i < len; i++)
    {
        DATA_S_List_append(&queue, &reg_frames[i]->node);
    }

    return 0;
}

int PCA9685PW::GetDutyCycle(uint8_t addr, uint16_t* value)
{
    //
    return 0;
}

int PCA9685PW::SetDutyCycle(uint8_t addr, uint16_t* value)
{
    return 0;
}

int PCA9685PW::FlushQueue()
{
    // Flush to bus
    if (queue.len == 0) return 0;
    if(dev.FlushQueue(&queue)) STD_FAIL;

    return 0;
}