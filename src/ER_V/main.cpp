#include <stdint.h>
#include <stdio.h>
#include <endian.h>
#include <string.h>

#include "util/comm.h"
#include "util/array.h"
#include "log/log.h"
#include "api/api.h"
#include "sub/sub.h"
#include "tamq/tamq_sub.h"
#include "arm/arm.h"
#include "erv_arm/erv.h"
#include "conf/config.h"
#include "erv_conf/erv_conf.h"
#include "tamq/tamq_conf.h"

#define LOG_FILE_THRESHOLD_THIS     LOG_THRESHOLD_MAX
#define LOG_CONSOLE_THRESHOLD_THIS  LOG_THRESHOLD_MAX

int main(int argc, char* argv[])
{
    // Initalize program; Setup Logging
    ERVConfig conf = ERVConfig();

    // Setup config priority
    const char* conf_loc[] = {NULL, CONF_DEFAULT_LOCATION};
    uint8_t conf_loc_len = UTIL_len(conf_loc);
    if (argc > 1) conf_loc[0] = argv[1];

    for (uint8_t iter = 0; iter < conf_loc_len; iter++)
        if(!conf.SetFilePath(conf_loc[iter]))
            break; // If file is successfully set, break loop

    conf.ParseConfig();

    LOG_init(conf.GetLogLocation());
    LOG_start();

    conf.DumpToLog(LOG_INFO);

    // Init Modules
    Subscriber hermes = Subscriber();
    SUB_Messenger* inbox = new TAMQ_Consumer(
        conf.GetBrokerAddress(),
        conf.GetCommandURI(),
        conf.GetUseTopics(),
        conf.GetClientAck());

    Arm* bot = new Scorbot(conf.GetScorbotDevicePath());

    // Register modules
    inbox->RegisterSubscriber(&hermes);
    bot->RegisterSubscriber(&hermes);

    // Start
    LOG_INFO("Staring demo...");
    hermes.Start();
    bot->Start();
    inbox->Start();

    // Loop
    while(1);

    // Cleanup running processes
    hermes.Stop();
    bot->Stop();

    // Release resources
    delete bot;
    bot = NULL;

    // End demo
    LOG_INFO("Demo Ending...");
    LOG_stop();
    LOG_destory();
}