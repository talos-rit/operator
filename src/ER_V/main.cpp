#include <stdint.h>
#include <stdio.h>
#include <endian.h>
#include <string.h>
#include <signal.h>
#include <execinfo.h>

#include "util/comm.h"
#include "util/array.h"
#include "log/log.h"
#include "sub/sub.h"
#include "tamq/tamq_sub.h"
#include "socket/socket.h"
#include "arm/arm.h"
#include "erv_arm/erv.h"
#include "conf/config.h"
#include "erv_conf/erv_conf.h"

#include "api/api.h"

#define LOG_FILE_THRESHOLD_THIS     LOG_THRESHOLD_MAX
#define LOG_CONSOLE_THRESHOLD_THIS  LOG_THRESHOLD_MAX

const char* app_name;
volatile int quit_sig = 0;

static void quit_handler(int signum)
{
    quit_sig = signum;   // quit control loop
}

static int register_intr()
{
    struct sigaction sa;
    sa.sa_handler = quit_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART; /* Restart functions if
                                 interrupted by handler */

    if (sigaction(SIGINT,  &sa, NULL) == -1) return -1;
    if (sigaction(SIGQUIT, &sa, NULL) == -1) return -1;
    if (sigaction(SIGABRT, &sa, NULL) == -1) return -1;
    return 0;
}

#if VALGRIND
static void dummy_msg(Subscriber* hermes)
{
    SUB_Buffer* buf = hermes->DequeueBuffer(SUB_QUEUE_FREE);
    if (!buf) return;

    API_Data_Wrapper* msg   = (API_Data_Wrapper*) &buf->body[0];

    msg->header.cmd_id      = htobe32(0x0);
    msg->header.reserved_1  = htobe16(0);
    msg->header.cmd_val     = htobe16(API_CMD_HOME);
    msg->header.len         = htobe16(sizeof(API_Data_Home));

    API_Data_Home* cmd      = (API_Data_Home*) &msg->payload_head;

    cmd->delay_ms = 0;

    buf->len = sizeof(API_Data_Home) + sizeof(API_Data_Header) + 2;
    hermes->EnqueueBuffer(SUB_QUEUE_COMMAND, buf);
}
#endif

int main(int argc, char* argv[])
{
    LOG_prep();
    app_name = argv[0];
    register_intr();

    // Initalize program; Setup Logging
    ERVConfig conf = ERVConfig();

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

    // Init Modules
    Subscriber hermes = Subscriber();
    // Inbox* inbox = new TAMQ_Consumer(
    //     conf.GetBrokerAddress(),
    //     conf.GetCommandURI(),
    //     conf.GetUseTopics(),
    //     conf.GetClientAck());
    Inbox* inbox = new Socket();

    Arm* bot = new Scorbot(conf.GetScorbotDevicePath());

    // Register modules
    inbox->RegisterSubscriber(&hermes);
    bot->RegisterSubscriber(&hermes);

    // Start
    hermes.Start();
    if(-1 == bot->Start()) quit_handler(SIGABRT);
    inbox->Start();

    // Loop
    if (!quit_sig) LOG_INFO("Ready.");
    while(!quit_sig);
    LOG_VERBOSE(0, "Quit signal: %d", quit_sig);
    LOG_INFO("Shutting down...");

    // Cleanup running processes
    hermes.Stop();
    bot->Stop();
    inbox->Stop();

    // Release resources
    delete bot;
    delete inbox;

    // End demo
    LOG_INFO("End Program.");
    LOG_stop();
    LOG_destory();
}