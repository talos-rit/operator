#pragma once
#include <pthread.h>

#include "sub/sub.h"
#include "data/s_list.h"


/** List of states the Inbox can be in */
// typedef enum _sub_status
// {
//     SUB_MODE_DEAD,
//     SUB_MODE_INIT,
//     SUB_MODE_RUN,
// } SUB_Status;

// /**
//  * @brief Initializes Subscriber
//  * @returns 0 on success, -1 on failure
// */
// int SUB_prep_subscriber ();

// /**
//  * @brief Deinitializes Subscriber
//  * @returns 0 on success, -1 on failure
// */
// int SUB_deinit_subscriber ();