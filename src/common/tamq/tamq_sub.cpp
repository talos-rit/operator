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

#include "tamq/tamq_sub.h"
#include "sub/sub.h"
#include "log/log.h"
#include "util/comm.h"
#include "util/array.h"

#define LOG_FILE_THRESHOLD_THIS     LOG_THRESHOLD_MAX
#define LOG_CONSOLE_THRESHOLD_THIS  LOG_THRESHOLD_DEFAULT

// Globally declare cleint variable
TAMQ_Consumer *client;
static int counter = 0;

TAMQ_Consumer::TAMQ_Consumer( const std::string& brokerURI,
                              const std::string& destURI,
                              bool useTopic,
                              bool clientAck) :
        connection(NULL),
        session(NULL),
        destination(NULL),
        consumer(NULL),
        useTopic(useTopic),
        brokerURI(brokerURI),
        destURI(destURI),
        clientAck(clientAck)
    {
        LOG_VERBOSE(4, "TAMQ Broker Address: %s",   brokerURI.c_str());
        LOG_VERBOSE(4, "TAMQ Command URI: %s",      destURI.c_str());
        LOG_VERBOSE(4, "TAMQ Use Topics: %s",       useTopic  ? "true" : "false");
        LOG_VERBOSE(4, "TAMQ Client ACK: %s",       clientAck ? "true" : "false");

        if (0 == counter++)
        {
            activemq::library::ActiveMQCPP::initializeLibrary();
        }
    }

TAMQ_Consumer::~TAMQ_Consumer() {
    if (0 == --counter)
    {
        activemq::library::ActiveMQCPP::shutdownLibrary();
    }
}

void TAMQ_Consumer::close() {
    this->cleanup();
}

void TAMQ_Consumer::runConsumer()
{
    try {

        // Create a ConnectionFactory
        activemq::core::ActiveMQConnectionFactory* connectionFactory =
            new activemq::core::ActiveMQConnectionFactory( brokerURI );

        // Create a Connection
        connection = connectionFactory->createConnection();
        delete connectionFactory;

        activemq::core::ActiveMQConnection* amqConnection = dynamic_cast<activemq::core::ActiveMQConnection*>( connection );
        if( amqConnection != NULL ) {
            amqConnection->addTransportListener( this );
        }

        connection->start();

        connection->setExceptionListener(this);

        // Create a Session
        if( clientAck ) {
            session = connection->createSession( cms::Session::CLIENT_ACKNOWLEDGE );
        } else {
            session = connection->createSession( cms::Session::AUTO_ACKNOWLEDGE );
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

    } catch (cms::CMSException& e) {
        e.printStackTrace();
    }
}

void TAMQ_Consumer::onMessage( const cms::Message* message )
{
    LOG_VERBOSE(2, "Received Message");
    int length = 0;

    try
    {
        SUB_Buffer *buf = sub->DequeueBuffer (SUB_QUEUE_FREE);
        if (!buf) STD_FAIL_VOID;

        const cms::BytesMessage* textMessage =
            dynamic_cast< const cms::BytesMessage* >( message );

        if( textMessage != NULL ) {
            length = textMessage->getBodyLength();
            memcpy(&buf->body, textMessage->getBodyBytes(), length);
        } else {
            length = 6;
            memcpy(&buf->body, "ERROR", length);
        }

        buf->len = length;

        if( clientAck ) {
            message->acknowledge();
        }

        #if LOG_CONSOLE_THRESHOLD_THIS >= LOG_VERBOSE + 6 | LOG_CONSOLE_THRESHOLD_THIS >= LOG_VERBOSE + 6
        char text[length * 6 + 5];
        sprintf(&text[0], "INIT");
        uint16_t str_iter = 0;
        for (uint16_t iter = 0; iter < length; iter++)
        {
            str_iter += sprintf(&text[str_iter], "0x%02X, ", buf->body[iter]);
        }

        LOG_VERBOSE(6, "Message : %d", length);
        #endif

        sub->EnqueueBuffer(SUB_QUEUE_COMMAND, buf);
    }

    catch (cms::CMSException& e)
    {
        e.printStackTrace();
    }
}

void TAMQ_Consumer::onException( const cms::CMSException& ex AMQCPP_UNUSED ) {
    LOG_ERROR("CMS Exception occurred.  Shutting down client.\n");
    exit(1);
}

void TAMQ_Consumer::transportInterrupted() {
    LOG_INFO("The Connection's Transport has been Interrupted.");
}

void TAMQ_Consumer::transportResumed() {
    LOG_INFO("The Connection's Transport has been Restored.");
}

int TAMQ_Consumer::Start()
{
    runConsumer(); // Start it up and it will listen forever.
    LOG_INFO("Talos ActiveMQ Client Running...");

    return 0;
}

int TAMQ_Consumer::Stop()
{
    // All CMS resources should be closed before the library is shutdown.
    close();
    LOG_INFO("Talos ActiveMQ Client Stopped");

    return 0;
}

int TAMQ_Consumer::RegisterSubscriber(Subscriber* sub)
{
    if (!sub) STD_FAIL;
    this->sub = sub;
    return 0;
}

void TAMQ_Consumer::cleanup(){

    try {
        if( connection != NULL ) {
            connection->close();
        }
    } catch ( cms::CMSException& e ) {
        e.printStackTrace();
    }

    delete destination;
    delete consumer;
    delete session;
    delete connection;
}