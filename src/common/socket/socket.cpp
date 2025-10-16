#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdint.h>
#include <sys/time.h>

#include "api/api.h"
#include "log/log.h"
#include "socket/socket.h"
#include "util/array.h"
#include "util/comm.h"

#define LOG_CONSOLE_THRESHOLD_THIS LOG_THRESHOLD_DEFAULT
#define LOG_FILE_THRESHOLD_THIS LOG_THRESHOLD_MAX

static int init_socket(Socket_Props *props) {
  // Set up socket
  props->sockfd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
  if (props->sockfd < 0) {
    LOG_ERROR("Could not open socket: (%d) %s", errno, strerror(errno));
    STD_FAIL;
  }

  int optval = 1;
  setsockopt(props->sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

  // Set up server address configuration
  memset(&props->server, 0, sizeof(props->server));
  props->port = 61616;
  props->server.sin_family = AF_INET;
  props->server.sin_addr.s_addr = INADDR_ANY;
  props->server.sin_port = htons(props->port);

  // Bind socket with server address configuration
  if (bind(props->sockfd, (struct sockaddr *)&props->server,
           sizeof(props->server)) < 0) {
    LOG_ERROR("Could not bind socket: (%d) %s", errno, strerror(errno));
    STD_FAIL;
  }

  return 0;
}

Socket::Socket() {
  // Create socket
  init_socket(&props);
  props.connfd = -1;
}

Socket::~Socket() {
  if (-1 != props.connfd)
    close(props.connfd);
  if (-1 != props.sockfd)
    close(props.sockfd);
}

static int wait_for_connection(Socket_Props *props) {
  // Accept connection
  socklen_t len = sizeof(props->client);
  LOG_INFO("Waiting for client...");

  while (props->thread_en && props->connfd < 0) {
    props->connfd =
        accept(props->sockfd, (struct sockaddr *)&props->client, &len);
    usleep(SOCKET_POLL_PERIOD_MS * 1000);
    if (-1 == props->connfd && errno != EAGAIN) {
      LOG_ERROR("Socket accept failed: (%d) %s", errno, strerror(errno));
      STD_FAIL;
    }
  }

  if (props->connfd < 0) {
    LOG_INFO("Connection Failed.");
    STD_FAIL;
  }

  LOG_INFO("Connection Established.");
  return 0;
}

int send_response(Socket_Props *props) {
  const char *response = "Message Received."; // placeholder response
  int send_ret = send(props->connfd, response, strlen(response), 0);

  if (send_ret < 0) {
    LOG_ERROR("Response failed to send.");
    STD_FAIL;
  } else {
    LOG_INFO("Response successfully sent");
    return 0;
  }
}

static void *socket_poll(void *arg) {
  Socket_Props *props = (Socket_Props *)arg;
  Subscriber *sub = props->sub;

  struct timeval last_ping;
  bool idle = true;
  int ret = 0;
  uint16_t buf_iter = 0;
  char buffer[SOCKET_BUF_LEN];

  memset(&buffer[0], 0, SOCKET_BUF_LEN);

  // Creates initial connection
  if (wait_for_connection(props) != 0) {
    LOG_ERROR("Connection initialization failed.");
    return NULL;
  }

  gettimeofday(&last_ping, NULL);
  while (props->thread_en) {
    if (idle)
      usleep(SOCKET_POLL_PERIOD_MS * 1000);
    idle = true;

    // Handle rx
    ret = recv(props->connfd, &buffer[buf_iter], sizeof(buffer) - buf_iter,
               MSG_DONTWAIT);
    if (-1 == ret) {
      // errno will be set to EAGAIN if no message was received
      if (EAGAIN == errno)
        continue;

      // Some other error; Exit execution
      LOG_FATAL("Socket read error: (%d) %s", errno, strerror(errno));
      LOG_IEC();
      break;
    }

    if (0 == ret) {
      // Getting to this point should mean that the connection closed
      // (otherwise recv would return -1 for a lack of a message)

      LOG_INFO("Connection closed by client.");

      close(props->connfd);
      props->connfd = -1;

      if (wait_for_connection(props) != 0) {
        LOG_ERROR("Reconnection failed.");
        break;
      }

      continue; // continue receiving after reconnection
    }

    // gettimeofday(&last_ping, NULL);
    idle = false;
    buf_iter += ret;

    while (buf_iter >= sizeof(API_Data_Header) + 2) {
      API_Data_Wrapper *msg = (API_Data_Wrapper *)&buffer[0];

      // Check length
      uint16_t len = sizeof(API_Data_Header) + be16toh(msg->header.len) + 2;
      if (buf_iter < len)
        break;

      // Enqueue copied message to command buffer
      SUB_Buffer *buf = sub->DequeueBuffer(SUB_QUEUE_FREE);
      if (!buf) {
        LOG_WARN("No free network buffers"); // You DDOS'd yourself.
        break;
      }

      buf->len = len;
      memcpy(&buf->body[0], msg, buf->len);
      sub->EnqueueBuffer(SUB_QUEUE_COMMAND, buf);
      LOG_VERBOSE(2, "Received ICD command");

      // Re-align buffer
      buf_iter -= len;
      memcpy(&buffer[0], &buffer[len], buf_iter);

      send_response(props);
    }
  }

  // Empty receive buffer; avoid TIME_WAIT on socket
  LOG_IEC();
  if (-1 == shutdown(props->connfd, SHUT_RDWR))
    LOG_ERROR("Error shutting down socket: (%d) %s", errno, strerror(errno));
  while (recv(props->connfd, &buffer[0], sizeof(buffer), 0) > 0)
    ;
  if (-1 != props->connfd) {
    close(props->connfd);
    props->connfd = -1;
  }

  return NULL;
}

int Socket::Start() {
  // Listen for connections
  listen(props.sockfd, 5);

  props.thread_en = true;
  pthread_create(&pid, NULL, socket_poll, &props);
  return 0;
}

int Socket::Stop() {
  props.thread_en = false;
  pthread_join(pid, NULL); // Wait for thread to exit
  return 0;
}

int Socket::RegisterSubscriber(Subscriber *sub) {
  if (!sub)
    STD_FAIL;
  this->sub = sub;
  props.sub = sub;
  return 0;
}