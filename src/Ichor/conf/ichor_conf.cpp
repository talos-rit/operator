#include <stdint.h>

#include "conf/config.h"
#include "conf/ichor_conf.h"
#include "util/comm.h"

#define LOG_CONSOLE_THRESHOLD_THIS LOG_THRESHOLD_DEFAULT
#define LOG_FILE_THRESHOLD_THIS LOG_THRESHOLD_DEFAULT

IchorConfig::IchorConfig() {
  AddKey(ICHOR_CONF_I2C_DEV_KEY, ICHOR_CONF_I2C_DEV_DEFAULT);
}

IchorConfig::~IchorConfig() {}

const char *IchorConfig::GetI2CDev() { return GetVal(ICHOR_CONF_I2C_DEV_KEY); }

int IchorConfig::LoadDefaults() { return 0; }