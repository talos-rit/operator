#pragma once

#include <stdint.h>

#include "conf/config.h"
#include "log/log_config.h"

#define ERV_CONF_DEV_KEY        "scorbot_dev"
#define ERV_CONF_DEV_DEFAULT    "/dev/ttyUSB0"

class ERVConfig:    public LogConfig
{
    private:
        uint8_t dev_idx;

    public:
        ERVConfig();
        virtual ~ERVConfig();

        /**
         * @brief Returns a const char pointer containing the linux device path of the Scorbot ER V serial connection
         * @returns const char pointer on success, NULL on failure
        */
        const char* GetScorbotDevicePath();

        /**
         * @brief Loads the ER V default configuration
         * @details Clear Key-Val table before using LoadDefaults\
         * @returns 0 on success, -1 on failure
        */
        int LoadDefaults();
};
