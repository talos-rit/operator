#include <stdint.h>
#include <stdio.h>
#include <endian.h>
#include <string.h>
#include <signal.h>
#include <execinfo.h>

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

const char* app_name;
volatile int loop = 1;

static void quit_handler(int signum)
{
    char deflt[4];
    const char* signal_name;

    loop = 0;   // quit control loop
    switch (signum)
    {
        case SIGQUIT:
            signal_name = "SIGQUIT";
            break;
        case SIGABRT:
            signal_name = "SIGABRT";
            break;
        default:
            sprintf(&deflt[0], "%d", signum);
            signal_name = (const char*) deflt;
            return;
    }

    LOG_VERBOSE(0, "Received Signal: %s; Quitting...", signal_name);
}

static void segv_handler(int signum)
{
    void *array[10];
    // char **messages = (char **)NULL;
    size_t size;

    // print out all the frames to stderr
    fprintf(stderr, "Error: signal %d:\n", signum);

    // get void*'s for all entries on the stack
    size = backtrace(array, 10);

    // messages = backtrace_symbols(array, size);
    backtrace_symbols_fd(array, size, STDERR_FILENO);

    for (uint8_t frame = 1; frame < size; frame++)
    {
        char syscom[256];
        sprintf(syscom,"addr2line -a -f --exe=%s +%p", app_name, array[frame]); //last parameter is the name of this app
        printf("CALL: %s\n", syscom);
        system(syscom);
    }

    exit(1);
}

int main(int argc, char* argv[])
{
    app_name = argv[0];

    // Configure interrupt
    struct sigaction sa;
    sa.sa_handler = quit_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART; /* Restart functions if
                                 interrupted by handler */

    if (sigaction(SIGQUIT, &sa, NULL) == -1) return -1;
    if (sigaction(SIGABRT, &sa, NULL) == -1) return -1;

    sa.sa_handler = segv_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART; /* Restart functions if
                                 interrupted by handler */

    if (sigaction(SIGSEGV, &sa, NULL) == -1) return -1;

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
    hermes.Start();
    if(-1 == bot->Start()) quit_handler(SIGABRT);
    inbox->Start();

    // Loop
    if (loop) LOG_INFO("Ready.");
    while(loop);
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