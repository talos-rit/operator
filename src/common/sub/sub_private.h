#pragma once
#include <pthread.h>

#include "sub/sub.h"
#include "data/s_list.h"


/** List of states the SUB_Messenger can be in */
typedef enum _sub_status
{
    SUB_MODE_DEAD,
    SUB_MODE_INIT,
    SUB_MODE_RUN,
} SUB_Status;

/** Resources for Subscriber messages */
typedef struct _sub_subcriber
{
    SUB_Messenger*  sub;                        /** Pointer to Concrete Subscriber instance */
    SUB_Status      status;                     /** Current status of subscriber interface */
    SUB_Concrete    sub_type;                   /** Stores the type of messenger being used (e.g.: TAMQ vs HTTP) */
    SUB_Buffer      msg_pool[SUB_MSG_COUNT];    /** Statically allocated pool of Sub Messages */
    pthread_mutex_t locks[SUB_QUEUE_LEN];       /** Subsciber queue locks; Indeces correspond to the SUB_Queue enum */
    S_List          queues[SUB_QUEUE_LEN];      /** Subsciber queues; Indeces correspond to the SUB_Queue enum */
} Subscriber;

/**
 * @brief Initializes Subscriber
 * @returns 0 on success, -1 on failure
*/
int SUB_prep_subscriber ();

/**
 * @brief Deinitializes Subscriber
 * @returns 0 on success, -1 on failure
*/
int SUB_deinit_subscriber ();