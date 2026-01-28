#include "erv_conf/erv_conf.hpp"

#include <stdint.h>

#include "conf/config.hpp"
#include "util/comm.hpp"

#define LOG_CONSOLE_THRESHOLD_THIS LOG_THRESHOLD_DEFAULT
#define LOG_FILE_THRESHOLD_THIS LOG_THRESHOLD_DEFAULT

ERVConfig::ERVConfig() {
  dev_idx = AddKey(ERV_CONF_DEV_KEY, ERV_CONF_DEV_DEFAULT);
}

ERVConfig::~ERVConfig() {}

const char* ERVConfig::GetScorbotDevicePath() { return GetVal(dev_idx); }

int ERVConfig::LoadDefaults() {
  dev_idx = AddKey(ERV_CONF_DEV_KEY, ERV_CONF_DEV_DEFAULT);
  return 0;
}