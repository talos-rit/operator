/**
 * Subscriber Module
 * Acts as the interface for the rest of the Operator system to send and receive messages.
 * This includes, logs, return messages, etc.
 * 
 * This interface is intended to be an agnostic interface to whatever messaging implementation is used to communicate with the director.
*/

#pragma once

#include <stdint.h>

#include "data/s_list.h"

#define SUB_MSG_LEN 255
#define SUB_MSG_COUNT 64
#define SUB_ADDR_LEN 32

/**
 * @class Subscriber Messenger Interface
 * @details Serves to separate the rest of the Talos Operator system from the messenger implementation (AMQ, HTTP, etc...)
*/
class SUB_Messenger 
{
private:
public:

    /**
     * @brief Constructor
    */
    SUB_Messenger() {}

    /**
     * @brief Destructor
    */
    virtual ~SUB_Messenger() {}

    /**
     * @brief Starts the Messenger service
     * @return 0 on success, -1 on failure
    */
    virtual int Start() = 0;

    /**
     * @brief Stops the Messenger service
     * @return 0 on success, -1 on failure
    */
    virtual int Stop() = 0;
};

#ifdef __cplusplus
extern "C" {
#endif

/** List of queues tracking Subscriber Buffers */
typedef enum _sub_queue
{
    SUB_QUEUE_FREE,     /** Index of Free Queue */
    SUB_QUEUE_COMMAND,  /** Index of Command Queue */
    SUB_QUEUE_RETURN,   /** Index of Return Queue */
    SUB_QUEUE_LOG,      /** Index of Log Queue */
    SUB_QUEUE_LEN,      /** Always last; Equal to length of enum */
} SUB_Queue;

/** List of implemented SUB_Messenger classes */
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

/**
 * @brief Configures and initializes Subscriber
 * @param type Specifies the implmentation type to use
 * @param sub_config Pointer to the implementation-specific configuration
*/
int SUB_init(SUB_Concrete type, void *sub_config);

/**
 * @brief Starts the Messenger service (C interface)
 * @return 0 on success, -1 on failure
*/
int SUB_start();

/**
 * @brief Stops the Messenger service (C interface)
 * @return 0 on success, -1 on failure
*/
int SUB_stop();

/**
 * @brief Destorys/Deinitializes the Messenger service
 * @return 0 on success, -1 on failure
*/
int SUB_destroy();

/**
 * @brief Initializes a Subscriber Buffer struct
 * @param buf Pointer to a SUB_Buffer pointer
 * @returns 0 on success, -1 on failure
*/
int SUB_init_buffer(SUB_Buffer *buf);

/**
 * @brief Dequeues a SUB_Buffer struct from the head of the specified list
 * @param queue Queue to dequeue from
 * @returns SUB_Buffer pointer on success, NULL on failure
*/
SUB_Buffer* SUB_dequeue_buffer(SUB_Queue queue);

/**
 * @brief Enqueues a SUB_Buffer struct from the head of the specified list
 * @param queue Queue to enqueue onto
 * @param buf Buffer to enqueue
 * @returns 0 on success, -1 on failure
*/
int SUB_enqueue_buffer(SUB_Queue queue, SUB_Buffer* buf);

#ifdef __cplusplus
}
#endif