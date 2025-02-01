#pragma once

#include <stdint.h>
#include <stddef.h>

#include "util/comm.h"
#include "data/list.h"
#include "data/s_list.h"

#define SERIAL_BUFFER_LEN 32
#define SERIAL_BUFFER_COUNT 32

class SerialDevice
{
public:
    /**
     * Used to track frames through their lifespan
    */
    typedef enum _serial_buffer_state
    {
        UNDEF,      /** Undefined; Indicates invalid state (uninitialized) */
        open,       /** Not being used and open to be taken */
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
    int InitFrame(SerialFrame* frame);

    /**
     * @brief Gets 1 or more frames from the open list of frames
     * @param count The number of frames to fetch
     * @returns A list of frames on success, NULL on failure
    */
    S_List* GetFrames(uint8_t count);

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

protected:

private:
    // Allocation
    SerialFrame* frame_pool;    // Holds the device's frame allocation
    uint8_t frame_count;        // Number of frames allocated

    // Queues
    S_List open;                // Frames that are kept here only if nothing else is using it and its not enqueued
    S_List queue;               // Frames here are waiting to be sent onto the bus;
};