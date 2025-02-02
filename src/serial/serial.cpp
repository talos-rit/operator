#include <stdlib.h>
#include <string.h>

#include "serial/serial.h"

#include "util/comm.h"
#include "log/log.h"

#define LOG_CONSOLE_THRESHOLD_THIS  LOG_THRESHOLD_DEFAULT
#define LOG_FILE_THRESHOLD_THIS     LOG_THRESHOLD_MAX

SerialDevice::SerialDevice()
{
    DATA_S_List_init(&free_queue_nodes);
    DATA_S_List_init(&queue);

    for(uint8_t iter = 0; iter < SERIAL_QUEUE_NODE_COUNT; iter++)
        InitQueueNode(&queue_nodes[iter]);
}

SerialDevice::~SerialDevice()
{
}

int SerialDevice::InitQueueNode(QueueNode* node)
{
    memset(node, 0, sizeof(QueueNode));
    DATA_S_List_Node_init(&node->node);
    DATA_S_List_append(&free_queue_nodes, &node->node);
}

int SerialDevice::InitFrame(SerialFrame* frame)
{
    if (!frame) STD_FAIL;
    memset(frame, 0, sizeof(SerialFrame));
    return 0;
}

int SerialDevice::QueueFrames(S_List* frames)
{
    if (!frames || frames->len == 0) STD_FAIL;

    S_List_Node* node = DATA_S_List_pop(&free_queue_nodes);
    DATA_LIST_GET_OBJ(node, QueueNode, node)->list = frames;
    DATA_S_List_append(&queue, node);
}