// (Talos) ActiveMQ implementation (TAMQ to avoid confusion with the Apache library)
#pragma once

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

int TAMQ_init();

int TAMQ_start();

int TAMQ_stop();

int TAMQ_destroy();