#include <stdint.h>
#include <string.h>

#include "log/log_config.h"
#include "conf/config.h"
#include "util/comm.h"

#define LOG_CONSOLE_THRESHOLD_THIS  LOG_THRESHOLD_MAX
#define LOG_FILE_THRESHOLD_THIS     LOG_THRESHOLD_MAX

LogConfig::LogConfig()
{
    log_loc_idx = AddKey(LOG_CONF_LOC_KEY, CONF_DATA_STRING);
}

LogConfig::~LogConfig()
{

}

int LogConfig::SetLogLocation(const char* log_location)
{
    if(!log_location) STD_FAIL;
    return OverrideValue(log_loc_idx, log_location);
}

const char* LogConfig::GetLogLocation()
{
    return GetVal(log_loc_idx);
}