/**
 * (Talos) ActiveMQ implementation (TAMQ to avoid confusion with the Apache library)
 * Implementation of the Subscriber interface
*/
#pragma once

#include <decaf/lang/Thread.h>
#include <decaf/lang/Runnable.h>
#include <decaf/util/concurrent/CountDownLatch.h>
#include <activemq/core/ActiveMQConnectionFactory.h>
#include <activemq/core/ActiveMQConnection.h>
#include <activemq/transport/DefaultTransportListener.h>
#include <activemq/library/ActiveMQCPP.h>
#include <decaf/lang/Integer.h>
#include <activemq/util/Config.h>
#include <decaf/util/Date.h>
#include <cms/Connection.h>
#include <cms/Session.h>
#include <cms/TextMessage.h>
#include <cms/BytesMessage.h>
#include <cms/MapMessage.h>
#include <cms/ExceptionListener.h>
#include <cms/MessageListener.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <string>

#include "util/comm.h"
#include "sub/sub.h"

#define TAMQ_CONFIG_FIELD_LEN 64

/** Talos ActiveMQ specific configurations */
typedef struct _tamq_config
{
    char connection[TAMQ_CONFIG_FIELD_LEN];     /** The IPv4 Address and port of the broker (e.g. 'failover:(tcp://127.0.0.1:61616)') */
    char dest_uri[TAMQ_CONFIG_FIELD_LEN];       /** The name of the topic/queue the client will listen to */
    bool use_topics;                            /** When true, TAMQ will consume from a topic, rather than a queue */
    bool client_ack;                            /** When true, TAMQ will not automatically send back an ack after consuming a message from the broker. */
} TAMQ_Configuration;

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

////////////////////////////////////////////////////////////////////////////////
class TAMQ_Consumer :   public cms::ExceptionListener,
                        public cms::MessageListener,
                        public activemq::transport::DefaultTransportListener,
                        public SUB_Messenger
{
    private:

        cms::Connection* connection;
        cms::Session* session;
        cms::Destination* destination;
        cms::MessageConsumer* consumer;
        bool useTopic;
        std::string brokerURI;      // IP Addr/Port to broker
        std::string destURI;        // Topic/Queue name
        bool clientAck;

    private:
        TAMQ_Consumer( const TAMQ_Consumer& );

    public:

        TAMQ_Consumer& operator= ( const TAMQ_Consumer& );
        TAMQ_Consumer( const std::string& brokerURI,
                            const std::string& destURI,
                            bool useTopic = false,
                            bool clientAck = false );

        virtual ~TAMQ_Consumer();
        void close();
        void runConsumer();

        // Called from the consumer since this class is a registered MessageListener.
        virtual void onMessage( const cms::Message* message );

        // If something bad happens you see it here as this class is also been
        // registered as an ExceptionListener with the connection.
        virtual void onException( const cms::CMSException& ex AMQCPP_UNUSED );
        virtual void transportInterrupted();
        virtual void transportResumed();
        int Start();
        int Stop();
        int RegisterSubscriber(Subscriber* sub);

    private:

        void cleanup();
};

////////////////////////////////////////////////////////////////////////////////