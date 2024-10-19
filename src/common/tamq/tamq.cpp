/*
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

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

#include "tamq/tamq.h"
#include "sub/sub.h"
#include "log/log.h"
#include "util/comm.h"
#include "util/array.h"

#define LOG_FILE_THRESHOLD_THIS LOG_THRESHOLD_MAX
#define LOG_CONSOLE_THRESHOLD_THIS LOG_VERBOSE + 6

using namespace activemq;
using namespace activemq::core;
using namespace activemq::transport;
using namespace decaf::lang;
using namespace decaf::util;
using namespace decaf::util::concurrent;
using namespace cms;
using namespace std;

// Globally declare cleint variable
class SimpleAsyncConsumer;
SimpleAsyncConsumer *client;

////////////////////////////////////////////////////////////////////////////////
class SimpleAsyncConsumer : public ExceptionListener,
                            public MessageListener,
                            public DefaultTransportListener,
                            public SUB_Messenger {
private:

    Connection* connection;
    Session* session;
    Destination* destination;
    MessageConsumer* consumer;
    bool useTopic;
    std::string brokerURI;      // IP Addr/Port to broker
    std::string destURI;        // Topic/Queue name
    bool clientAck;
    unsigned char buff[512];    // Buffer for holding the message

private:
    SimpleAsyncConsumer( const SimpleAsyncConsumer& );

public:

    SimpleAsyncConsumer& operator= ( const SimpleAsyncConsumer& );
    SimpleAsyncConsumer( const std::string& brokerURI,
                         const std::string& destURI,
                         bool useTopic = false,
                         bool clientAck = false ) :
        connection(NULL),
        session(NULL),
        destination(NULL),
        consumer(NULL),
        useTopic(useTopic),
        brokerURI(brokerURI),
        destURI(destURI),
        clientAck(clientAck) {
    }

    virtual ~SimpleAsyncConsumer() {
        this->cleanup();
    }

    void close() {
        this->cleanup();
    }

    void runConsumer() {

        try {

            // Create a ConnectionFactory
            ActiveMQConnectionFactory* connectionFactory =
                new ActiveMQConnectionFactory( brokerURI );

            // Create a Connection
            connection = connectionFactory->createConnection();
            delete connectionFactory;

            ActiveMQConnection* amqConnection = dynamic_cast<ActiveMQConnection*>( connection );
            if( amqConnection != NULL ) {
                amqConnection->addTransportListener( this );
            }

            connection->start();

            connection->setExceptionListener(this);

            // Create a Session
            if( clientAck ) {
                session = connection->createSession( Session::CLIENT_ACKNOWLEDGE );
            } else {
                session = connection->createSession( Session::AUTO_ACKNOWLEDGE );
            }

            // Create the destination (Topic or Queue)
            if( useTopic ) {
                destination = session->createTopic( destURI );
            } else {
                destination = session->createQueue( destURI );
            }

            // Create a MessageConsumer from the Session to the Topic or Queue
            consumer = session->createConsumer( destination );
            consumer->setMessageListener( this );

        } catch (CMSException& e) {
            e.printStackTrace();
        }
    }

    // Called from the consumer since this class is a registered MessageListener.
    virtual void onMessage( const Message* message ) 
    {
        LOG_VERBOSE(2, "Received Message");
        static int count = 0;
        int length = 0; 

        try
        {
            count++;
            const BytesMessage* textMessage =
                dynamic_cast< const BytesMessage* >( message );

            if( textMessage != NULL ) {
                length = textMessage->getBodyLength();
                memcpy(&buff[0], textMessage->getBodyBytes(), length);
            } else {
                length = 6;
                memcpy(&buff, "ERROR", length);
            }

            if( clientAck ) {
                message->acknowledge();
            }

            char text[length * 6 + 5];
            sprintf(&text[0], "INIT");
            uint16_t str_iter = 0;
            for (uint16_t iter = 0; iter < length; iter++)
            {
                str_iter += sprintf(&text[str_iter], "0x%02X, ", buff[iter]);
            }

            LOG_VERBOSE(4, "Byte length: %d", length);
            LOG_VERBOSE(6, "Message #%d Received: Length: %d, Message: %s", count, length, text);
        }

        catch (CMSException& e) 
        {
            e.printStackTrace();
        }
    }

    // If something bad happens you see it here as this class is also been
    // registered as an ExceptionListener with the connection.
    virtual void onException( const CMSException& ex AMQCPP_UNUSED ) {
        LOG_ERROR("CMS Exception occurred.  Shutting down client.\n");
        exit(1);
    }

    virtual void transportInterrupted() {
        LOG_INFO("The Connection's Transport has been Interrupted.");
    }

    virtual void transportResumed() {
        LOG_INFO("The Connection's Transport has been Restored.");
    }

    int Start() 
    {
        client->runConsumer(); // Start it up and it will listen forever.
        LOG_INFO("Talos ActiveMQ Client Running...");

        return 0;
    }

    int Stop() 
    {
        // All CMS resources should be closed before the library is shutdown.
        client->close();
        LOG_INFO("Talos ActiveMQ Client Stopped");

        return 0;
    }

private:

    void cleanup(){

        try {
            if( connection != NULL ) {
                connection->close();
            }
        } catch ( CMSException& e ) { 
            e.printStackTrace(); 
        }

        delete destination;
        delete consumer;
        delete session;
        delete connection;
    }
};

////////////////////////////////////////////////////////////////////////////////
SUB_Messenger* TAMQ_init(TAMQ_Config *config) 
{
    activemq::library::ActiveMQCPP::initializeLibrary();
    TAMQ_Config conf;

    // Copy configuration
    if (NULL != config) memcpy(&conf, config, sizeof(TAMQ_Config));
    else
    {
        conf = {TAMQ_BROKER_URI, TAMQ_DEST_URI, TAMQ_USE_TOPICS, TAMQ_CLIENT_ACK};
    }

    client = new SimpleAsyncConsumer (  conf.connection, 
                                        conf.dest_uri, 
                                        conf.use_topics, 
                                        conf.client_ack ); // Create the consumer


    LOG_VERBOSE(0, "Talos ActiveMQ Client Initialized");
    return client;
}

int TAMQ_destroy()
{
    delete client;
    activemq::library::ActiveMQCPP::shutdownLibrary();
    LOG_VERBOSE(0, "Talos ActiveMQ Client Stopped");

    return 0;
}