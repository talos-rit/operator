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

#define LOG_FILE_THRESHOLD_THIS     LOG_THRESHOLD_MAX
#define LOG_CONSOLE_THRESHOLD_THIS  LOG_THRESHOLD_MAX

int main()
{
    // Initalize program; Setup Logging
    LOG_init();
    LOG_start();

    // Init Modules
    Subscriber hermes = Subscriber();
    SUB_Messenger* inbox = new TAMQ_Consumer(TAMQ_BROKER_URI, TAMQ_DEST_URI, TAMQ_USE_TOPICS, TAMQ_CLIENT_ACK);
    Arm* bot = new Scorbot("/dev/ttyUSB1");

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