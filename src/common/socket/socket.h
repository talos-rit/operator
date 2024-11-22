#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>

#include "sub/sub.h"

#define SOCKET_BUF_LEN 1024
#define SOCKET_POLL_PERIOD_MS 25

typedef struct _sock_props
{
    int sockfd = -1;
    int connfd = -1;
    int port;
    struct sockaddr_in server;
    struct sockaddr_in client;
    Subscriber *sub;
}Socket_Props;

class Socket : public Inbox
{
    private:
        int status = 0;
        pthread_t pid = -1;

        Socket_Props props;

        void* Poll(void* arg);

    public:
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

        virtual int RegisterSubscriber(Subscriber* sub) = 0;
};