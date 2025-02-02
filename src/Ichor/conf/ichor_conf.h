#pragma once

#include <stdint.h>

#include "conf/config.h"
#include "log/log_config.h"
#include "tamq/tamq_conf.h"

#define ICHOR_CONF_I2C_DEV_KEY        "i2c_dev"
#define ICHOR_CONF_I2C_DEV_DEFAULT    "/dev/i2c-1"

class IchorConfig:  public LogConfig
                    // ,public TAMQ_Config
{
    private:

    public:
        IchorConfig();
        virtual ~IchorConfig();

        const char* GetI2CDev();

        /**
         * @brief Loads the ER V default configuration
         * @details Clear Key-Val table before using LoadDefaults\
         * @returns 0 on success, -1 on failure
        */
        int LoadDefaults();
};