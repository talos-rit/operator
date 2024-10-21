#include <stdint.h>
#include <stdio.h>
#include <endian.h>
#include <string.h>

#include "util/comm.h"
#include "util/array.h"
#include "log/log.h"
#include "api/api.h"
#include "sub/sub.h"
#include "arm/arm.h"
#include "erv_arm/erv.h"

#define LOG_FILE_THRESHOLD_THIS     LOG_THRESHOLD_MAX
#define LOG_CONSOLE_THRESHOLD_THIS  LOG_THRESHOLD_MAX

int main()
{
    LOG_init();
    LOG_start();
    LOG_INFO("Staring demo...");

    SUB_init(SUB_MSG_AMQ, NULL);
    Scorbot bot = Scorbot("/dev/ttyUSB1");

    SUB_start();
    bot.Start();

    while(1);

    bot.Stop();
    SUB_stop();
    SUB_destroy();

    LOG_INFO("Demo Ending...");
    LOG_stop();
    LOG_destory();
}