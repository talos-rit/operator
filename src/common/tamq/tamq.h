/**
 * (Talos) ActiveMQ implementation (TAMQ to avoid confusion with the Apache library)
 * Implementation of the Subscriber interface
*/
#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <string>

#include "util/comm.h"
#include "sub/sub.h"

using namespace std;

// Set the URI to point to the IPAddress of your broker.
// add any optional params to the url to enable things like
// tightMarshalling or tcp logging etc.  See the CMS web site for
// a full list of configuration options.
//
//  http://activemq.apache.org/cms/
//
#define TAMQ_BROKER_URI "failover:(tcp://127.0.0.1:61616)"

//============================================================
// This is the Destination Name and URI options.  Use this to
// customize where the consumer listens, to have the consumer
// use a topic or queue set the 'useTopics' flag.
//============================================================
#define TAMQ_DEST_URI "operator_0"

//============================================================
// set to true to use topics instead of queues
// Note in the code above that this causes createTopic or
// createQueue to be used in the consumer.
//============================================================
#define TAMQ_USE_TOPICS false

//============================================================
// set to true if you want the consumer to use client ack mode
// instead of the default auto ack mode.
//============================================================
#define TAMQ_CLIENT_ACK false

#define TAMQ_CONFIG_FIELD_LEN 64

/** Talos ActiveMQ specific configurations */
typedef struct _tamq_config
{
    char connection[TAMQ_CONFIG_FIELD_LEN];     /** The IPv4 Address and port of the broker (e.g. 'failover:(tcp://127.0.0.1:61616)') */
    char dest_uri[TAMQ_CONFIG_FIELD_LEN];       /** The name of the topic/queue the client will listen to */
    bool use_topics;                            /** When true, TAMQ will consume from a topic, rather than a queue */
    bool client_ack;                            /** When true, TAMQ will not automatically send back an ack after consuming a message from the broker. */
} TAMQ_Config;

/** ID numbers of various AMQ message types */
typedef enum _tamq_message_type
{
    ID_ACTIVEMQBLOBMESSAGE      = 29,
    ID_ACTIVEMQBYTESMESSAGE     = 24,
    ID_ACTIVEMQMAPMESSAGE       = 25,
    ID_ACTIVEMQMESSAGE          = 23,
    ID_ACTIVEMQOBJECTMESSAGE    = 26,
    ID_ACTIVEMQSTREAMMESSAGE    = 27,
    ID_ACTIVEMQTEXTMESSAGE      = 28,
} Tamq_Message_Type;

/**
 * @brief Initializes TAMQ as a SUB_Messenger
 * @param config Pointer to a config to use to initialize the TAMQ
 * @returns SUB_Messeneger pointer on success, NULL on failure
*/
SUB_Messenger *TAMQ_init(TAMQ_Config *config);

/**
 * @brief Destroys/Deinitializes TAMQ
 * @returns 0 on success, -1 on failure
*/
int TAMQ_destroy();