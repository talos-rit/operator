#include <stdio.h>

#include "log/log.h"
#include "sub/sub.h"

#define LOG_FILE_THRESHOLD_THIS     LOG_THRESHOLD_MAX
#define LOG_CONSOLE_THRESHOLD_THIS  LOG_THRESHOLD_DEFAULT

int main()
{
    LOG_init();
    LOG_start();

    SUB_init(SUB_MSG_AMQ, NULL);
    SUB_start();

    while(1);

    SUB_stop();
    SUB_destroy();
}