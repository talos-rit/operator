/**
 * Subscriber Module
 * Acts as the interface for the rest of the Operator system to send and receive messages.
 * This includes, logs, return messages, etc.
 *
 * This interface is intended to be an agnostic interface to whatever messaging implementation is used to communicate with the director.
*/

#pragma once

#include <stdint.h>
#include <pthread.h>

#include "data/s_list.h"
#include "util/comm.h"
#include "sub/sub.h"
#include "sub/inbox.h"

#define SUB_MSG_LEN 255
#define SUB_MSG_COUNT 64
#define SUB_ADDR_LEN 32

/** List of queues tracking Subscriber Buffers */
typedef enum _sub_queue
{
    SUB_QUEUE_FREE,     /** Index of Free Queue */
    SUB_QUEUE_COMMAND,  /** Index of Command Queue */
    SUB_QUEUE_RETURN,   /** Index of Return Queue */
    SUB_QUEUE_LOG,      /** Index of Log Queue */
    SUB_QUEUE_LEN,      /** Always last; Equal to length of enum */
} SUB_Queue;

/** List of implemented Inbox classes */
typedef enum _sub_msg_implementaion
{
    SUB_MSG_AMQ,
} SUB_Concrete;

/** Subscriber Buffer */
typedef struct _sub_buf
{
    uint8_t     body[SUB_MSG_LEN];  /** Message contents */
    uint8_t     len;                /** Message length */
    S_List_Node node;               /** Tracks buffer through Subscriber Queues */
} SUB_Buffer;

/** Resources for Subscriber messages */
typedef struct _sub_instance
{
    // Inbox*  cmd;                        /** Pointer to Concrete Subscriber instance */
    // SUB_Status      status;                     /** Current status of subscriber interface */
    // SUB_Concrete    sub_type;                   /** Stores the type of messenger being used (e.g.: TAMQ vs HTTP) */
    // SUB_Buffer      msg_pool[SUB_MSG_COUNT];    /** Statically allocated pool of Sub Messages */
    uint32_t        pool_len;                   /** Number of SUB_Buffers in the message pool */
    SUB_Buffer*     msg_pool;                   /** Dynamically allocated pool of Sub Messages */
    pthread_mutex_t locks[SUB_QUEUE_LEN];       /** Subscriber queue locks; Indeces correspond to the SUB_Queue enum */
    S_List          queues[SUB_QUEUE_LEN];      /** Subscriber queues; Indeces correspond to the SUB_Queue enum */
} SUB_Instance;


/**
 * @brief Initializes a Subscriber Buffer struct
 * @param buf Pointer to a SUB_Buffer pointer
 * @returns 0 on success, -1 on failure
*/
int SUB_init_buffer(SUB_Buffer *buf);

class Subscriber
{
    private:
    enum class SUB_State
    {
        DEAD,
        INIT,
        RUN,
    };

    SUB_State status;           /** Current status of subscriber interface */
    SUB_Instance resources;     /** Resources for managing buffers */

    public:
    Subscriber();
    ~Subscriber();

    int Start();
    int Stop();
    void Abort();

    int RegisterResponder(Inbox* responder);

    // TODO: implement the following:
    // int RegisterResponder(Inbox* responder);
    // int RegisterLogger(Inbox* logger);

    /**
     * @brief Enqueues a SUB_Buffer struct from the head of the specified list
     * @param queue Queue to enqueue onto
     * @param buf Buffer to enqueue
     * @returns 0 on success, -1 on failure
    */
    int EnqueueBuffer(SUB_Queue queue_idx, SUB_Buffer* buf);

    /**
     * @brief Dequeues a SUB_Buffer struct from the head of the specified list
     * @param queue Queue to dequeue from
     * @returns SUB_Buffer pointer on success, NULL on failure
    */
    SUB_Buffer* DequeueBuffer(SUB_Queue queue_idx);

};