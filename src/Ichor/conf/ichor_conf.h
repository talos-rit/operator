#pragma once

#include <stdint.h>

#include "conf/config.h"
#include "log/log_config.h"
#include "tamq/tamq_conf.h"

class IchorConfig:  public LogConfig
                    // ,public TAMQ_Config
{
    private:

    public:
        IchorConfig();
        virtual ~IchorConfig();

        /**
         * @brief Loads the ER V default configuration
         * @details Clear Key-Val table before using LoadDefaults\
         * @returns 0 on success, -1 on failure
        */
        int LoadDefaults();
};