#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

#include "arm/arm.h"
#include "api/api.h"
#include "log/log.h"
#include "util/comm.h"

#define LOG_CONSOLE_THRESHOLD_THIS  LOG_THRESHOLD_MAX
#define LOG_FILE_THRESHOLD_THIS     LOG_THRESHOLD_MAX

Arm* arm = NULL;

Arm::Arm()
{
    if (arm) return;
    thread_enable = false;
    arm = this;
}

Arm::~Arm()
{
    arm = NULL;
}

bool Arm::GetThreadEnable()
{
    return this->thread_enable;
}

static void* ARM_run(void*)
{
    // TODO: Add exit code
    while(1)
    {
        SUB_Buffer *buf = SUB_dequeue_buffer(SUB_QUEUE_COMMAND);
        if (!buf)
        {
            // arm->Poll();
            if (!arm->GetThreadEnable()) break;
            usleep(25*1000);
            continue;
        }

        arm->ProcessBuffer(buf);
    }

    return 0;
}

int Arm::Start()
{
    thread_enable = true;
    int ret = pthread_create(&this->pid, NULL, ARM_run, NULL);
    if (ret == -1) STD_FAIL;
    return 0;
}

int Arm::Stop()
{
    STD_FAIL;
}

int Arm::ProcessBuffer(SUB_Buffer *buf)
{
    if (!buf) STD_FAIL;
    if (API_validate_command(&buf->body[0], buf->len)) STD_FAIL;
    API_Data_Wrapper *cmd = (API_Data_Wrapper *) &buf->body;

    int status = 0;
    switch (cmd->header.cmd_val)
    {
        case API_CMD_HANDSHAKE:
            LOG_INFO("Handshake Recieved");
            if (HandShake()) STD_FAIL;
            break;
        case API_CMD_POLARPAN:
            LOG_INFO("Polar Pan Recieved");
            if (PolarPan((API_Data_Polar_Pan *) &cmd->payload_head)) STD_FAIL;
            break;
        case API_CMD_HOME:
            LOG_INFO("Home Received");
            if (Home((API_Data_Home *)&cmd->payload_head)) STD_FAIL;
            break;
        default:
            LOG_IEC();
            status = -1;
            break;
    }

    SUB_init_buffer(buf);
    SUB_enqueue_buffer(SUB_QUEUE_FREE, buf);
    return status;
}
