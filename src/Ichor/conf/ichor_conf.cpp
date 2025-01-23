#include <stdint.h>

#include "conf/ichor_conf.h"
#include "conf/config.h"
#include "util/comm.h"

#define LOG_CONSOLE_THRESHOLD_THIS  LOG_THRESHOLD_DEFAULT
#define LOG_FILE_THRESHOLD_THIS     LOG_THRESHOLD_DEFAULT

IchorConfig::IchorConfig()
{
}

IchorConfig::~IchorConfig()
{

}

int IchorConfig::LoadDefaults()
{
    return 0;
}