#pragma once

#include "conf/config.h"

#define SOCKET_CONF_ADDR_KEY        "socket_address"
#define SOCKET_CONF_ADDR_DEFAULT    "0.0.0.0"
#define SOCKET_CONF_PORT_KEY        "socket_port"
#define SOCKET_CONF_PORT_DEFAULT    61616

class Socket_Config: virtual public Config
{
    private:
        int address_idx;    /** IP of the ActiveMQ broker */
        int cmd_uri_idx;    /** Topic/Queue for incoming commands */
        int ret_uri_idx;    /** Topic/Queue for return messages */
        int log_uri_idx;    /** Topic/Queue for logs */
        int use_topics_idx; /** Determines if topics will be used, rather than queues */
        int client_ack_idx; /** Determines if client will send ACK message, or if it will automatically be sent */

    public:
        Socket_Config();
        ~Socket_Config();

        /**
         * @brief Gets config for ActiveMQ broker address
         * @returns config string on success, NULL on failure
        */
        const char* GetBrokerAddress();

        /**
         * @brief Sets config for ActiveMQ broker address
         * @param addr Broker address to set in config
         * @returns 0 on success, -1 on failure
        */
        int SetAddress(const char* addr);

        int LoadDefaults();
};