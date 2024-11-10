#pragma once

#include "conf/config.h"

#define LOG_CONF_LOC_KEY        "log_loc"
#define LOG_CONF_LOC_DEFAULT    "/etc/talos/logs/operator.log"

class LogConfig: virtual public Config
{
    private:
        int log_loc_idx;    /** Where to put the log file generated by the logging module */

    public:
        LogConfig();
        ~LogConfig();

        const char* GetLogLocation();
        int SetLogLocation(const char* log_location);
};