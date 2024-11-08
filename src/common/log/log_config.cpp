#include <stdint.h>
#include <string.h>

#include "log/log_config.h"
#include "conf/config.h"
#include "util/comm.h"

#define LOG_CONSOLE_THRESHOLD_THIS  LOG_THRESHOLD_MAX
#define LOG_FILE_THRESHOLD_THIS     LOG_THRESHOLD_MAX

int LogConfig::SetLogLocation(const char* log_location)
{
    if(!log_location) STD_FAIL;
    if(strlen(log_location) + 1 > CONF_MEMBER_LEN)
    {
        LOG_WARN("Configured log location is too long; switching to default.");
        SetLogLocation(CONF_DEFAULT_LOCATION);
        return -1;
    }
    return 0;
}

const char* LogConfig::GetLogLocation()
{
    return (const char*) log_loc;
}