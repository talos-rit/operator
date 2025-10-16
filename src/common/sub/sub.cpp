#include "sub/sub.h"

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "api/api.h"
#include "data/list.h"
#include "data/s_list.h"
#include "log/log.h"
#include "sub/sub_private.h"
#include "util/array.h"
#include "util/comm.h"

#define LOG_CONSOLE_THRESHOLD_THIS LOG_THRESHOLD_DEFAULT
#define LOG_FILE_THRESHOLD_THIS LOG_THRESHOLD_MAX

#define SUB_ABORT \
  {               \
    LOG_IEC();    \
    Abort();      \
    return -1;    \
  }

/**
 * @brief Helper function to initialize Subscriber queues' locks and lists
 * @param queue_idx SUB_Queue value corresponding to a specific queue's index in
 * the array of queues
 * @returns 0 on success, -1 on failure
 */
static int init_queue(SUB_Instance &resources, SUB_Queue queue_idx) {
  // If the message pool is NULL (unallocated), allocate a new pool.
  // If the message pool is not NULL, assume it has been allocated already
  // (length is compile time constant)
  if (!resources.msg_pool)
    resources.msg_pool =
        (SUB_Buffer *)malloc(SUB_MSG_COUNT * sizeof(SUB_Buffer));

  if (pthread_mutex_init(&resources.locks[(uint8_t)queue_idx], NULL)) STD_FAIL
  if (DATA_S_List_init(&resources.queues[(uint8_t)queue_idx])) STD_FAIL

  return 0;
}

/**
 * @brief Helper function to deinitialize Subscriber queues' locks and lists
 * @param queue_idx SUB_Queue value corresponding to a specific queue's index in
 * the array of queues
 * @returns 0 on success, -1 on failure
 */
static int deinit_queue(SUB_Instance &resources, SUB_Queue queue_idx) {
  pthread_mutex_t *mutex = &resources.locks[(uint8_t)queue_idx];

  // Block until mutex is available, then unlock to destroy
  // Threads should be disabled by this point
  pthread_mutex_lock(mutex);
  pthread_mutex_unlock(mutex);

  if (pthread_mutex_destroy(&resources.locks[(uint8_t)queue_idx])) STD_FAIL
  if (DATA_S_List_deinit(&resources.queues[(uint8_t)queue_idx])) STD_FAIL

  return 0;
}

static int SUB_prep_subscriber(SUB_Instance &resources) {
  memset(&resources, 0, sizeof(SUB_Instance));

  for (uint8_t iter = 0; iter < SUB_QUEUE_LEN; iter++)
    init_queue(resources, (SUB_Queue)iter);

  pthread_mutex_lock(&resources.locks[SUB_QUEUE_FREE]);
  for (uint16_t iter = 0; iter < SUB_MSG_COUNT; iter++) {
    SUB_Buffer *buf = &resources.msg_pool[iter];
    SUB_init_buffer(buf);
    DATA_S_List_append(&resources.queues[SUB_QUEUE_FREE], &buf->node);
  }
  pthread_mutex_unlock(&resources.locks[SUB_QUEUE_FREE]);

  return 0;
}

static int SUB_deinit_subscriber(SUB_Instance &resources) {
  for (uint8_t iter = 0; iter < SUB_QUEUE_LEN; iter++)
    if (deinit_queue(resources, (SUB_Queue)iter)) STD_FAIL;

  if (resources.msg_pool) {
    free(resources.msg_pool);
    resources.msg_pool = NULL;
  }

  return 0;
}

Subscriber::Subscriber() {
  LOG_INFO("Subscriber Initializing...");

  SUB_prep_subscriber(resources);
  status = SUB_State::INIT;
  LOG_INFO("Subscriber Initialized.");
}

void Subscriber::Abort() {}

Subscriber::~Subscriber() {
  if (SUB_State::INIT != status) LOG_IEC();
  status = SUB_State::DEAD;
  SUB_deinit_subscriber(resources);
}

int Subscriber::Start() {
  if (SUB_State::INIT != status) STD_FAIL;

  /*
  // TODO: Implement responder
  if (!responder) SUB_ABORT;
  if (responder->Start()) STD_FAIL;
  */

  status = SUB_State::RUN;
  return 0;
}

int Subscriber::RegisterResponder(Inbox *responder) {
  if (!responder) STD_FAIL;
  if (SUB_State::RUN == status)
    LOG_WARN("Registering a responder on a running Subscriber.");

  this->responder = responder;
  return 0;
}

int Subscriber::Stop() {
  if (SUB_State::RUN != status) STD_FAIL;
  status = SUB_State::INIT;
  return 0;
}

int SUB_init_buffer(SUB_Buffer *buf) {
  if (!buf) STD_FAIL;

  memset(buf, 0, sizeof(SUB_Buffer));
  DATA_S_List_Node_init(&buf->node);
  return 0;
}

SUB_Buffer *Subscriber::DequeueBuffer(SUB_Queue queue_idx) {
  if ((uint8_t)queue_idx >= SUB_MSG_LEN) STD_FAIL_VOID_PTR;

  S_List_Node *node = NULL;
  pthread_mutex_lock(&resources.locks[(uint8_t)queue_idx]);
  node = DATA_S_List_pop(&resources.queues[(uint8_t)queue_idx]);
  pthread_mutex_unlock(&resources.locks[(uint8_t)queue_idx]);

  if (!node) return NULL;
  LOG_VERBOSE(2, "SUB_Buffer dequeued from queue index #%d",
              (uint8_t)queue_idx);
  return DATA_LIST_GET_OBJ(node, SUB_Buffer, node);
}

int Subscriber::EnqueueBuffer(SUB_Queue queue_idx, SUB_Buffer *buf) {
  if (SUB_State::RUN != status)
    LOG_WARN("Enqueueing a buffer onto a stopped Subscriber.");
  if (!buf) STD_FAIL;
  if ((uint8_t)queue_idx >= SUB_MSG_LEN) STD_FAIL;

  if (SUB_QUEUE_FREE == queue_idx) SUB_init_buffer(buf);
  pthread_mutex_lock(&resources.locks[(uint8_t)queue_idx]);
  int ret =
      DATA_S_List_append(&resources.queues[(uint8_t)queue_idx], &buf->node);
  pthread_mutex_unlock(&resources.locks[(uint8_t)queue_idx]);

  if (ret) STD_FAIL;
  LOG_VERBOSE(2, "SUB_Buffer enqueued in queue index #%d", (uint8_t)queue_idx);
  return 0;
}
