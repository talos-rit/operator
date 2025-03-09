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

#include "gpio/pin.h"
#include "gpio/isr.h"

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

    /*************************************************************/
    /************************START SANDBOX************************/
    /*************************************************************/

    uint8_t enc_a = 24, enc_b = 25;
    RotaryEncoder enc = RotaryEncoder();
    GPIO_INTR_TARGET target = {.enc = &enc};
    IchorISR isr = IchorISR("/dev/gpiochip0");

    isr.RegisterPin(enc_a, GPIO_INTR_TYPE_ENCODER_A, target);
    isr.RegisterPin(enc_b, GPIO_INTR_TYPE_ENCODER_B, target);
    isr.AllocatePins();

    usleep(5.0e6);                          // Sleep 5s
    isr.ProcessEvents();                    // Process events stored in queue by GPIO kernel driver
    int32_t val = enc.GetValue();           // Check how encoder has changed

    LOG_INFO("Encoder value: %d", val);

    /*************************************************************/
    /*************************END SANDBOX*************************/
    /*************************************************************/

    // End demo
    LOG_INFO("End Program.");
    LOG_stop();
    LOG_destory();
}
