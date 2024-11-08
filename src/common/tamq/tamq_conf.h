#pragma once

#include "conf/config.h"

class TAMQ_Config: public Config
{
    private:
        char address[CONF_MEMBER_LEN];      /** IP of the ActiveMQ broker  */
        char cmd_uri[CONF_MEMBER_LEN];      /** Topic/Queue for incoming commands */
        char resp_uri[CONF_MEMBER_LEN];     /** Topic/Queue for return messages */
        char log_uri[CONF_MEMBER_LEN];      /** Topic/Queue for logs */

    public:
        TAMQ_Config();
        ~TAMQ_Config();

        const char* GetBrokerAddress();
        int SetBrokerAddress(const char* addr);
};