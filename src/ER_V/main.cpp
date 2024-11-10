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

int main()
{
    // Initalize program; Setup Logging
    ERVConfig conf = ERVConfig();
    conf.SetFilePath(CONF_DEFAULT_LOCATION);
    conf.ParseConfig();

    LOG_init(conf.GetLogLocation());
    LOG_start();


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