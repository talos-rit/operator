#include "log/log.h"
#include "tamq/tamq.h"

#define LOG_FILE_THRESHOLD_THIS     LOG_THRESHOLD_MAX
#define LOG_CONSOLE_THRESHOLD_THIS  LOG_THRESHOLD_DEFAULT

int main()
{
    LOG_init();
    LOG_start();

    TAMQ_init();
    TAMQ_start();

    while(1);

    LOG_stop();
    LOG_destory();
}