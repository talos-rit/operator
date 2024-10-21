#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "sub/sub.h"
#include "sub/sub_private.h"
#include "util/comm.h"
#include "util/array.h"
#include "log/log.h"
#include "tamq/tamq.h"
#include "data/list.h"
#include "data/s_list.h"

#include "api/api.h"

#define LOG_CONSOLE_THRESHOLD_THIS LOG_THRESHOLD_MAX
#define LOG_FILE_THRESHOLD_THIS LOG_THRESHOLD_MAX

Subscriber hermes;

/**
 * @brief Helper function to initialize Subscriber queues' locks and lists
 * @param queue_idx SUB_Queue value corresponding to a specific queue's index in the array of queues
 * @returns 0 on success, -1 on failure
*/
static int init_queue(SUB_Queue queue_idx)
{
    if (pthread_mutex_init(&hermes.locks[(uint8_t) queue_idx],  NULL)) STD_FAIL
    if (DATA_S_List_init(&hermes.queues[(uint8_t) queue_idx])) STD_FAIL 
    
    return 0;
}

/**
 * @brief Helper function to deinitialize Subscriber queues' locks and lists
 * @param queue_idx SUB_Queue value corresponding to a specific queue's index in the array of queues
 * @returns 0 on success, -1 on failure
*/
static int deinit_queue(SUB_Queue queue_idx)
{
    pthread_mutex_t *mutex = &hermes.locks[(uint8_t) queue_idx];

    // Block until mutex is available, then unlock to destroy
    // Threads should be disabled by this point
    pthread_mutex_lock(mutex);
    pthread_mutex_unlock(mutex);

    if (pthread_mutex_destroy(&hermes.locks[(uint8_t) queue_idx]))  STD_FAIL
    if (DATA_S_List_deinit(&hermes.queues[(uint8_t) queue_idx]))    STD_FAIL

    return 0;
}

int SUB_prep_subscriber()
{
    memset(&hermes, 0, sizeof(Subscriber));
    hermes.status = SUB_MODE_DEAD;

    for (uint8_t iter = 0; iter < SUB_QUEUE_LEN; iter++)
        init_queue((SUB_Queue) iter);

    pthread_mutex_lock(&hermes.locks[SUB_QUEUE_FREE]);
    for (uint16_t iter = 0; iter < SUB_MSG_COUNT; iter++)
        SUB_init_buffer (&hermes.msg_pool[iter]);
    pthread_mutex_unlock(&hermes.locks[SUB_QUEUE_FREE]);

    return 0;
}

int SUB_deinit_subscriber()
{
    for (uint8_t iter = 0; iter < SUB_QUEUE_LEN; iter++)
        if (deinit_queue((SUB_Queue) iter)) STD_FAIL;

    return 0;
}

int SUB_init(SUB_Concrete type, void *sub_config)
{
    LOG_INFO("Subscriber Initializing...");
    SUB_prep_subscriber();
    hermes.sub_type = type;
    
    switch(type)
    {
        case SUB_MSG_AMQ:
            //TODO: Create configuration logic
            hermes.sub = TAMQ_init((TAMQ_Config*) sub_config);
            break;
        default:
            STD_FAIL;
            break;
    }

    hermes.status = SUB_MODE_INIT;
    LOG_INFO("Subscriber Initialized.");
    return 0;
}

int SUB_destroy()
{
    if (SUB_MODE_INIT != hermes.status) STD_FAIL;
    
    switch(hermes.sub_type)
    {
        case SUB_MSG_AMQ:
            TAMQ_destroy();
            break;
        default:
            STD_FAIL;
            break;
    }

    hermes.status = SUB_MODE_DEAD;

    return 0;
}

int SUB_start()
{
    if (SUB_MODE_INIT != hermes.status) STD_FAIL;
    if (hermes.sub->Start()) STD_FAIL;

    hermes.status = SUB_MODE_RUN;
    return 0;
}

int SUB_stop()
{
    if (SUB_MODE_RUN != hermes.status) STD_FAIL;
    if (hermes.sub->Stop()) STD_FAIL;

    hermes.status = SUB_MODE_INIT;
    return 0;
}

int SUB_init_buffer(SUB_Buffer *buf)
{
    if (!buf) STD_FAIL;

    memset(buf, 0, sizeof(SUB_Buffer));
    DATA_S_List_Node_init (&buf->node);
    DATA_S_List_append(&hermes.queues[SUB_QUEUE_FREE], &buf->node);

    return 0;
}

SUB_Buffer* SUB_dequeue_buffer(SUB_Queue queue_idx)
{
    if (SUB_MODE_RUN != hermes.status)      STD_FAIL_VOID_PTR;
    if ((uint8_t) queue_idx >= SUB_MSG_LEN)  STD_FAIL_VOID_PTR;

    S_List_Node* node = NULL;
    pthread_mutex_lock      (&hermes.locks [(uint8_t) queue_idx]);
    node = DATA_S_List_pop  (&hermes.queues[(uint8_t) queue_idx]);
    pthread_mutex_unlock    (&hermes.locks [(uint8_t) queue_idx]);
    
    if (!node) return NULL;
    LOG_VERBOSE(2, "SUB_Buffer dequeued from queue index #%d", (uint8_t) queue_idx);
    return DATA_LIST_GET_OBJ(node, SUB_Buffer, node);
}

int SUB_enqueue_buffer(SUB_Queue queue_idx, SUB_Buffer* buf)
{
    if (SUB_MODE_RUN != hermes.status) STD_FAIL;
    if (!buf) STD_FAIL;
    if ((uint8_t) queue_idx >= SUB_MSG_LEN) STD_FAIL;

    #if 0
    API_validate_command(&buf->body[0], buf->len);
    API_Data_Wrapper* cmd   = (API_Data_Wrapper*)   &buf->body;
    char text[255];
    uint8_t iter = 0;
    API_Data_Polar_Pan* pan = (API_Data_Polar_Pan*) &cmd->payload_head;

    switch (cmd->header.cmd_val)
    {
        case API_CMD_POLARPAN:
            iter += sprintf(&text[iter], "Polar Pan\n");
            iter += sprintf(&text[iter], "\tΔ Azimuth: \t%d\n",     pan->delta_azimuth);
            iter += sprintf(&text[iter], "\tΔ Altitude: \t%d\n",    pan->delta_altitude);
            iter += sprintf(&text[iter], "\tDelay: \t\t%d\n",       pan->delay_ms);
            iter += sprintf(&text[iter], "\tTime: \t\t%d\n",        pan->time_ms);
            break;
        default:
            iter += sprintf(&text[iter], "Unrecognized Command Value: %d", cmd->header.cmd_val);
            break;
    }

    LOG_VERBOSE(4, "Message Recieved: %s", text);
    queue_idx = SUB_QUEUE_FREE;
    #endif

    if (SUB_QUEUE_FREE == queue_idx) SUB_init_buffer (buf);

    pthread_mutex_lock      (&hermes.locks [(uint8_t) queue_idx]);
    int ret = DATA_S_List_append (&hermes.queues[(uint8_t) queue_idx], &buf->node);
    if (ret) LOG_IEC();
    pthread_mutex_unlock    (&hermes.locks [(uint8_t) queue_idx]);
    
    if (ret) return -1;
    LOG_VERBOSE(2, "SUB_Buffer enqueued in queue index #%d", (uint8_t) queue_idx);
    return 0;
}