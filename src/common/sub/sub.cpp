#include <stdlib.h>
#include <stdint.h>

#include "sub/sub.h"
#include "util/comm.h"
#include "log/log.h"
#include "tamq/tamq.h"

#define LOG_CONSOLE_THRESHOLD_THIS LOG_THRESHOLD_DEFAULT
#define LOG_FILE_THRESHOLD_THIS LOG_THRESHOLD_MAX

SUB_Messenger *hermes = NULL;
SUB_Mode mode = SUB_MODE_DEAD;
SUB_Concrete msg_type;

int SUB_init(SUB_Concrete type, void *sub_config)
{
    if (SUB_MODE_DEAD != mode) STD_FAIL;
    msg_type = type;
    
    switch(type)
    {
        case SUB_MSG_AMQ:
            //TODO: Create configuration logic
            hermes = TAMQ_init((TAMQ_Config*) sub_config);
            break;
        default:
            STD_FAIL;
            break;
    }

    mode = SUB_MODE_INIT;
    return 0;
}

int SUB_destroy()
{
    if (SUB_MODE_INIT != mode) STD_FAIL;
    
    switch(msg_type)
    {
        case SUB_MSG_AMQ:
            TAMQ_destroy();
            break;
        default:
            STD_FAIL;
            break;
    }
}

int SUB_start()
{
    if (SUB_MODE_INIT != mode) STD_FAIL;
    hermes->Start();
}

int SUB_stop()
{
    if (SUB_MODE_RUN != mode) STD_FAIL;
    hermes->Stop();
}