#include "log/log.h"

#define LOG_FILE_THRESHOLD_THIS LOG_THRESHOLD_MAX
#define LOG_CONSOLE_THRESHOLD_THIS LOG_THRESHOLD_DEFAULT

int main() {
  LOG_init();
  LOG_start();

  LOG_FATAL("Fatal error");
  LOG_ERROR("Standard error");
  LOG_IEC();
  LOG_WARN("Warning");
  LOG_INFO("Information");
  LOG_VERBOSE(0, "A little extra info");
  LOG_VERBOSE(1, "Debugging");
  LOG_VERBOSE(2, "A lot of debugging");
  LOG_VERBOSE(9, "Dear god, the debugging");

  LOG_stop();
  LOG_destory();
}