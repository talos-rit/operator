#include <stdint.h>
#include <errno.h>

#include "socket/socket.h"
#include "util/comm.h"
#include "log/log.h"
#include "api/api.h"

#define LOG_CONSOLE_THRESHOLD_THIS  LOG_THRESHOLD_MAX
#define LOG_FILE_THRESHOLD_THIS     LOG_THRESHOLD_MAX

static int init_socket(Socket_Props* props)
{
    props->sockfd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
    if (props->sockfd < 0)
    {
        LOG_ERROR("Could not open socket: (%d) %s", errno, strerror(errno));
        STD_FAIL;
    }

    // Set up server address
    memset(&props->server, 0, sizeof(props->server));
    props->port = 61616;
    props->server.sin_family = AF_INET;
    props->server.sin_addr.s_addr = INADDR_ANY;
    props->server.sin_port = htons(props->port);

    if (bind(props->sockfd, (struct sockaddr *) &props->server, sizeof(props->server)) < 0)
    {
        LOG_ERROR("Could not bind socket: (%d) %s", errno, strerror(errno));
        STD_FAIL;
    }

    return 0;
}

static int abort_connection(Socket_Props* props)
{
    // LOG_VERBOSE(4, "Close socket: %d", close(props->sockfd)); // Unblocks accept syscall
    // shutdown(props->sockfd, SHUT_RDWR);

    // props->sockfd = -1;
    // return init_socket(props); // reinit socket for new accept
    LOG_IEC();
    return 0;
}

Socket::Socket()
{
    // Create socket
    init_socket(&props);
    props.connfd = -2;
}

Socket::~Socket()
{
    status = init_socket(&props);
}

static void* socket_poll (void* arg)
{
    Socket_Props *props = (Socket_Props*) arg;
    Subscriber* sub = props->sub;

    int ret = 0;
    int buf_iter = 0;
    int buf_len = 0;
    char buffer[SOCKET_BUF_LEN];
    bool idle = true;

    // Accept connection
    socklen_t len = sizeof(props->client);
    LOG_INFO("Waiting for client...");

    while(props->thread_en && props->connfd < 0)
    {
        props->connfd = accept(props->sockfd, (struct sockaddr *) &props->client, &len);
        usleep(1000);
        if (-1 == props->connfd && errno != EAGAIN)
        {
            LOG_ERROR("Socket accept failed: (%d) %s", errno, strerror(errno));
            STD_FAIL_VOID_PTR;
        }
    }

    if (props->connfd > 0) LOG_INFO ("Connection Established.");
    else LOG_INFO("Connection Failed.");

    while(props->thread_en)
    {
        if (idle) usleep (SOCKET_POLL_PERIOD_MS * 1000);
        idle = true;

        // Handle rx
        ret = read(props->connfd, &buffer[buf_iter], sizeof(buffer) - buf_iter);
        if (-1 == ret)
        {
            LOG_IEC();
            break;
        }

        if (0 == ret) continue;

        buf_len += ret;
        LOG_VERBOSE(4, "Ret: %d", ret);
        while (buf_len > 0)
        {
            LOG_VERBOSE(4, "Socket received data");
            buffer[buf_len] = '\0';
            LOG_VERBOSE(6, "Message: %s", &buffer[0]);
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
            if (sizeof(msg->header) + msg->header.len + 2 > (uint16_t) ret)
            {
                // Incomplete message
                idle = true;
                if(buf_iter != 0) memcpy(&buffer[0], &buffer[buf_iter], buf_len);
                buf_iter = buf_len;
                break;
            }

            rx->len = sizeof(msg->header) + msg->header.len + 2;
            memcpy(&rx->body[0], &msg, rx->len);
            rx->body[rx->len] = '\0';
            buf_iter += rx->len;
            buf_len  -= rx->len;
            LOG_VERBOSE(2, "Socket received: %s", &rx->body[0]);
            sub->EnqueueBuffer(SUB_QUEUE_COMMAND, rx);
        }
    }

    return NULL;
}

int Socket::Start()
{
    // Listen for connections
    listen(props.sockfd, 5);

    props.thread_en = true;
    pthread_create(&pid, NULL, socket_poll, &props);
    return 0;
}

int Socket::Stop()
{
    props.thread_en = false;
    if (-2 == props.connfd) abort_connection(&props);   // Accept call is still blocking; abort
    if (-1 != props.connfd) close(props.connfd);        // Otherwise, close connection
    pthread_join(pid, NULL);                            // Wait for thread to exit
    return 0;
}

int Socket::RegisterSubscriber(Subscriber* sub)
{
    if (!sub) STD_FAIL;
    this->sub = sub;
    props.sub = sub;
    return 0;
}