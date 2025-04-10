#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

#include "arm/arm.h"
#include "api/api.h"
#include "log/log.h"
#include "util/comm.h"

#define LOG_CONSOLE_THRESHOLD_THIS  LOG_THRESHOLD_MAX
#define LOG_FILE_THRESHOLD_THIS     LOG_THRESHOLD_MAX

Arm::Arm()
{
    thread_enable = false;
}

Arm::~Arm()
{
}

bool Arm::GetThreadEnable()
{
    return this->thread_enable;
}

static void* ARM_run(void* arg)
{
    Arm* arm = (Arm*) arg;

    // TODO: Add exit code
    while(1)
    {
        int ret = arm->ProcessQueue();
        if (1 == ret)
        {
            arm->Poll();
            if (!arm->GetThreadEnable()) break;
            usleep(ARM_POLL_PERIOD_MS * 1000);
            continue;
        }
    }

    return 0;
}

int Arm::Start()
{
    thread_enable = true;
    int ret = pthread_create(&this->pid, NULL, ARM_run, this);
    if (ret == -1) STD_FAIL;
    return 0;
}

int Arm::Stop()
{
    this->thread_enable = 0;
    pthread_join(pid, NULL);
    return 0;
}

int Arm::RegisterSubscriber(Subscriber* sub)
{
    if (!sub) STD_FAIL;
    this->sub = sub;
    return 0;
}

int Arm::ProcessQueue()
{
    SUB_Buffer* buf = sub->DequeueBuffer(SUB_QUEUE_COMMAND);

    if (!buf) return 1;
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
        case API_CMD_POLARPAN_START:
            LOG_INFO("Polar Pan Start Recieved");
            if (PolarPanStart((API_Data_Polar_Pan_Start *) &cmd->payload_head)) STD_FAIL;
            break;
        case API_CMD_POLARPAN_STOP:
            LOG_INFO("Polar Pan Stop Recieved");
            if (PolarPanStop()) STD_FAIL;
            break;
        case API_CMD_CARTESIAN_START:
            LOG_INFO("Cartesian Start Recieved");
            if (CartesianStart((API_Data_Cartesian_Start *) &cmd->payload_head)) STD_FAIL;
            break;
        case API_CMD_CARTESIAN_STOP:
            LOG_INFO("Cartesian Stop Recieved");
            if (CartesianStop()) STD_FAIL;
            break;
        default:
            LOG_IEC();
            status = -1;
            break;
    }

    sub->EnqueueBuffer(SUB_QUEUE_FREE, buf);
    return status;
}
