#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "sub/sub.h"
#include "sub/sub_private.h"
#include "util/comm.h"
#include "log/log.h"
#include "tamq/tamq.h"
#include "data/list.h"
#include "data/s_list.h"

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
    
    if (!node) STD_FAIL_VOID_PTR;
    LOG_VERBOSE(2, "SUB_Buffer dequeued from queue index #%d", (uint8_t) queue_idx);
    return DATA_LIST_GET_OBJ(node, SUB_Buffer, node);
}

int SUB_enqueue_buffer(SUB_Queue queue_idx, SUB_Buffer* buf)
{
    if (SUB_MODE_RUN != hermes.status) STD_FAIL;
    if (!buf) STD_FAIL;
    if ((uint8_t) queue_idx >= SUB_MSG_LEN) STD_FAIL;

    #if 1
    uint16_t length = buf->len;
    char text[length * 6 + 5];
    sprintf(&text[0], "INIT");
    uint16_t str_iter = 0;
    for (uint16_t iter = 0; iter < length; iter++)
    {
        str_iter += sprintf(&text[str_iter], "0x%02X, ", buf->body[iter]);
    }

    LOG_VERBOSE(6, "Message : %s", text);
    queue_idx = SUB_QUEUE_FREE;
    #endif

    pthread_mutex_lock      (&hermes.locks [(uint8_t) queue_idx]);
    if(DATA_S_List_append   (&hermes.queues[(uint8_t) queue_idx], &buf->node)) STD_FAIL;
    pthread_mutex_unlock    (&hermes.locks [(uint8_t) queue_idx]);
    
    LOG_VERBOSE(2, "SUB_Buffer enqueued in queue index #%d", (uint8_t) queue_idx);

    return 0;
}