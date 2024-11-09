#include <stdint.h>

#include "erv_conf/erv_conf.h"
#include "conf/config.h"
#include "util/comm.h"

#define LOG_CONSOLE_THRESHOLD_THIS  LOG_THRESHOLD_DEFAULT
#define LOG_FILE_THRESHOLD_THIS     LOG_THRESHOLD_DEFAULT

ERVConfig::ERVConfig()
{
    dev_idx = AddKey(ERV_CONF_DEV_KEY);
}

ERVConfig::~ERVConfig()
{

}

const char* ERVConfig::GetScorbotDevicePath()
{
    return GetVal(dev_idx);
}

int ERVConfig::LoadDefaults()
{
    dev_idx = AddKey(ERV_CONF_DEV_KEY);
    if(OverrideValue(dev_idx, ERV_CONF_DEV_DEFAULT)) STD_FAIL;
    return 0;
}