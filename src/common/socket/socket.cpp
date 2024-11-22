#include <stdint.h>
#include <errno.h>

#include "socket/socket.h"
#include "util/comm.h"
#include "log/log.h"
#include "api/api.h"

#define LOG_CONSOLE_THRESHOLD_THIS  LOG_THRESHOLD_MAX
#define LOG_FILE_THRESHOLD_THIS     LOG_THRESHOLD_MAX

Socket::Socket()
{
    // Create socket
    props.sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (props.sockfd < 0)
    {
        LOG_ERROR("Could not open socket: (%d) %s", errno, strerror(errno));
        status = -1;
        return;
    }

    // Set up server address
    memset(&props.server, 0, sizeof(props.server));
    props.port = 61616;
    props.server.sin_family = AF_INET;
    props.server.sin_addr.s_addr = INADDR_ANY;
    props.server.sin_port = htons(props.port);

    if (bind(props.sockfd, (struct sockaddr *) &props.server, sizeof(props.server)) < 0)
    {
        LOG_ERROR("Could not bind socket: (%d) %s", errno, strerror(errno));
        status = -1;
        return;
    }
}

Socket::~Socket()
{
    if (-1 != props.sockfd) close(props.sockfd);
}

static void* socket_poll (void* arg)
{
    Socket_Props *props = (Socket_Props*) arg;
    Subscriber* sub = props->sub;
    SUB_Buffer* tx = NULL;

    int ret = 0;
    int buf_iter = 0;
    int buf_len = 0;
    char buffer[SOCKET_BUF_LEN];
    bool idle = true;

    while(1)
    {
        if (idle) usleep (SOCKET_POLL_PERIOD_MS * 1000);
        idle = true;

        // Handle rx
        ret = read(props->connfd, &buffer[buf_iter], sizeof(buffer) - buf_iter);
        if (-1 == ret) STD_FAIL_VOID;
        buf_len += ret;
        while (buf_len > 0)
        {
            // Received data
            idle = false;

            SUB_Buffer* rx = NULL;
            if (!(rx = sub->DequeueBuffer(SUB_QUEUE_FREE)))
            {
                // No free buffers for receiving
                LOG_WARN("Socket ran out of free subscriber buffers");
                continue;
            }

            API_Data_Wrapper* msg = (API_Data_Wrapper*) &buffer[buf_iter];
            if (sizeof(msg->header) + msg->header.len + 2 > ret)
            {
                // Incomplete message
                idle = true;
                if(buf_iter != 0) memcpy(&buffer[0], &buffer[buf_iter], buf_len);
                buf_iter = buf_len;
                break;
            }

            rx->len = sizeof(msg->header) + msg->header.len + 2;
            memcpy(&rx->body[0], &msg, rx->len);
            buf_iter += rx->len;
            buf_len  -= rx->len;

            sub->EnqueueBuffer(SUB_QUEUE_COMMAND, rx);
        }
    }
}

int Socket::Start()
{
    // Listen for connections
    listen(props.sockfd, 5);
    socklen_t len = sizeof(props.client);

    // Accept connection
    props.connfd = accept(props.sockfd, (struct sockaddr *) &props.client, &len);
    if (-1 == props.connfd)
        LOG_ERROR("Socket accept failed: (%d) %s", errno, strerror(errno));

    pthread_create(&pid, NULL, socket_poll, sub);
}

int Socket::Stop()
{
    if (-1 != props.connfd) close(props.connfd);
}

int Socket::RegisterSubscriber(Subscriber* sub)
{
    this->sub = sub;
    props.sub = sub;
}