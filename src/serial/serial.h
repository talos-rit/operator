#pragma once

#include <stdint.h>
#include <stddef.h>

#include "util/comm.h"
#include "data/list.h"
#include "data/s_list.h"

#define SERIAL_QUEUE_NODE_COUNT 32

class SerialDevice
{
private:
    // Internal queue management stuff
    typedef struct queue_node
    {
        S_List* list;
        S_List_Node node;
    } QueueNode;

    int InitQueueNode(QueueNode* node);

    QueueNode queue_nodes[SERIAL_QUEUE_NODE_COUNT]; // Used to tie queues together without mixing
    S_List free_queue_nodes;    // List of free queues to use
    S_List queue;               // Lists of frames wait here to be tranceived on the bus;

protected:

public:
    /**
     * Used to track frames through their lifespan
    */
    typedef enum _serial_buffer_state
    {
        IDLE,       /** Waiting to be enqueued */
        ENQUEUED,   /** Enqueued to be processed */
        PROCESSED,  /** Finished processing; waiting to be reinitialized/used again */
    } SerialState;

    /**
     * Implementation agnostic data frame for transmission.
     * Can handle simple, half duplex, or full duplex communication.
    */
    typedef struct _serial_frame
    {
        uint8_t* tx_buf;        /** Buffer to read transmitted data from */
        uint8_t* rx_buf;        /** Buffer to put received data in*/
        uint8_t len;            /** Number of bytes of transaction */
        uint32_t delay_ms;      /** Delay in milliseconds between this frame and the next */

        void* callback(void);   /** Callback function for when transaction is complete */
        SerialState state;      /** The node's current state*/
        S_List_Node node;       /** Used to link frames into a list */
    } SerialFrame;

    SerialDevice();
    ~SerialDevice();

    /**
     * @brief Initializes a SerialFrame to its default state
     * @param frame A pointer to the frame to initialize
     * @returns 0 on success, -1 on failure
    */
    static int InitFrame(SerialFrame* frame);

    /**
     * @brief Queues 1 or more SerialFrames to be executed on the bus
     * @param frames Singlely linked list of frames to enqueue
    */
    int QueueFrames(S_List* frames);

    /**
     * @brief Flushed the queue list of frames to the communication bus
     * @returns 0 on success, -1 on failure
    */
    virtual int FlushQueue() = 0;
};