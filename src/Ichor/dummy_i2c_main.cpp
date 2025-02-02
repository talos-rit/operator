#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <fcntl.h>
#include <endian.h>
#include <string.h>
#include <signal.h>
#include <execinfo.h>

#include "util/comm.h"
#include "util/array.h"
#include "log/log.h"
#include "conf/config.h"
#include "serial/test/dummy_i2c.h"

#include "arm/ichor_arm.h"
#include "conf/ichor_conf.h"

#define LOG_FILE_THRESHOLD_THIS     LOG_THRESHOLD_MAX
#define LOG_CONSOLE_THRESHOLD_THIS  LOG_THRESHOLD_MAX

int main(int argc, char* argv[])
{
    LOG_prep();

    // Initalize program; Setup Logging
    IchorConfig conf = IchorConfig();

    // Setup config priority
    const char* conf_loc[] = {NULL, CONF_DEFAULT_LOCATION};
    uint8_t conf_loc_len = UTIL_len(conf_loc);
    if (argc > 1) conf_loc[0] = argv[1];

    for (uint8_t iter = 0; iter < conf_loc_len; iter++)
        if(conf_loc[iter] && !conf.SetFilePath(conf_loc[iter]))
            break; // If file is successfully set, break loop

    conf.ParseConfig();

    LOG_init(conf.GetLogLocation());
    LOG_start();

    conf.DumpToLog(LOG_INFO);

    int fd = open(conf.GetI2CDev(), O_RDWR);
    if (fd < 0) LOG_WARN("Failed to open I2C bus");
    DummyI2C* dummy = new DummyI2C(fd, 0x80);

    const uint8_t msg[] = {0xDE, 0xAD, 0xBE, 0xEF};
    LOG_INFO("Echo result: %d", dummy->Echo(15, msg, 4));

    // End demo
    LOG_INFO("End Program.");
    LOG_stop();
    LOG_destory();
}