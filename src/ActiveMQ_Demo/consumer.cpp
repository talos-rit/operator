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

using namespace activemq;
using namespace activemq::core;
using namespace activemq::transport;
using namespace decaf::lang;
using namespace decaf::util;
using namespace decaf::util::concurrent;
using namespace cms;
using namespace std;

////////////////////////////////////////////////////////////////////////////////
class TAMQ_Consumer : public ExceptionListener,
                            public MessageListener,
                            public DefaultTransportListener {
private:

    Connection* connection;
    Session* session;
    Destination* destination;
    MessageConsumer* consumer;
    bool useTopic;
    std::string brokerURI;
    std::string destURI;
    bool clientAck;

    char buff[512];

private:

    TAMQ_Consumer( const TAMQ_Consumer& );
    TAMQ_Consumer& operator= ( const TAMQ_Consumer& );

public:

    TAMQ_Consumer( const std::string& brokerURI,
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

    virtual ~TAMQ_Consumer() {
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
    virtual void onMessage( const Message* message ) {

        static int count = 0;

        try
        {
            count++;
            const BytesMessage* textMessage =
                dynamic_cast< const BytesMessage* >( message );
            unsigned char* bytes = {0};

            if( textMessage != NULL ) {
                bytes = textMessage->getBodyBytes();
            } else {
                bytes = {0};
            }

            if( clientAck ) {
                message->acknowledge();
            }

            int iter = 0;
            printf("Length: %d\n", textMessage->getBodyLength());
            for (int i = 0; i < textMessage->getBodyLength(); i++)
            {
                printf("i: %d\n", i);
                iter += sprintf(&buff[iter], "0x%02X, ", bytes[i]);
            }

            printf( "Message #%d Received: %s\n", count, buff );
        } catch (CMSException& e) {
            e.printStackTrace();
        }
    }

    // If something bad happens you see it here as this class is also been
    // registered as an ExceptionListener with the connection.
    virtual void onException( const CMSException& ex AMQCPP_UNUSED ) {
        printf("CMS Exception occurred.  Shutting down client.\n");
        exit(1);
    }

    virtual void transportInterrupted() {
        std::cout << "The Connection's Transport has been Interrupted." << std::endl;
    }

    virtual void transportResumed() {
        std::cout << "The Connection's Transport has been Restored." << std::endl;
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
int main(int argc AMQCPP_UNUSED, char* argv[] AMQCPP_UNUSED) {

    activemq::library::ActiveMQCPP::initializeLibrary();

    std::cout << "=====================================================\n";
    std::cout << "Starting the example:" << std::endl;
    std::cout << "-----------------------------------------------------\n";

    // Set the URI to point to the IPAddress of your broker.
    // add any optional params to the url to enable things like
    // tightMarshalling or tcp logging etc.  See the CMS web site for
    // a full list of configuration options.
    //
    //  http://activemq.apache.org/cms/
    //
    std::string brokerURI =
        // "failover:(tcp://192.168.120.115:61616)";
        "failover:(tcp://127.0.0.1:61616)";

    //============================================================
    // This is the Destination Name and URI options.  Use this to
    // customize where the consumer listens, to have the consumer
    // use a topic or queue set the 'useTopics' flag.
    //============================================================
    std::string destURI = "TEST.FOO"; //?consumer.prefetchSize=1";

    //============================================================
    // set to true to use topics instead of queues
    // Note in the code above that this causes createTopic or
    // createQueue to be used in the consumer.
    //============================================================
    bool useTopics = false;

    //============================================================
    // set to true if you want the consumer to use client ack mode
    // instead of the default auto ack mode.
    //============================================================
    bool clientAck = false;

    // Create the consumer
    TAMQ_Consumer consumer( brokerURI, destURI, useTopics, clientAck );

    // Start it up and it will listen forever.
    consumer.runConsumer();

    // Wait to exit.
    std::cout << "Press 'q' to quit" << std::endl;
    while( std::cin.get() != 'q') {}

    // All CMS resources should be closed before the library is shutdown.
    consumer.close();

    std::cout << "-----------------------------------------------------\n";
    std::cout << "Finished with the example." << std::endl;
    std::cout << "=====================================================\n";

    activemq::library::ActiveMQCPP::shutdownLibrary();
}