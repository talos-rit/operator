#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <fcntl.h>
#include <endian.h>
#include <string.h>
#include <signal.h>
#include <execinfo.h>
#include <sys/time.h>
#include <math.h>

#include "util/comm.h"
#include "util/array.h"
#include "log/log.h"
#include "conf/config.h"

#include "dac/PCA9685PW.h"
#include "gpio/isr.h"
#include "driver/driver.h"

#include "arm/ichor_arm.h"
#include "conf/ichor_conf.h"

#define LOG_FILE_THRESHOLD_THIS     LOG_THRESHOLD_MAX
#define LOG_CONSOLE_THRESHOLD_THIS  LOG_THRESHOLD_MAX

#define ENC_A       24
#define ENC_B       25
#define DAC_IN1     4
#define DAC_IN2     3
#define DAC_SPEED   2
#define ADC_CHANNEL -1

float velocity_square (int32_t start, int32_t stop, int32_t pos)
{
    if (stop == pos) return 0;
    return (stop - pos > 0 ? 1 : -1);
}

float velocity_sine (int32_t start, int32_t stop, int32_t pos)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);

    float tmp = ((tv.tv_sec % 10) * 1e6 + tv.tv_usec) / 1e6f;
    tmp = sin(tmp * M_PI / 5.f);
    // tmp *= DAC_PCA_MAX_DUTY_CYCLE;
    return tmp;
}

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

    int fd = open(conf.GetI2CDev(), O_RDWR);
    if (fd < 0) LOG_WARN("Failed to open I2C bus");
    PCA9685PW dac = PCA9685PW(fd, 0x60);
    IchorISR isr = IchorISR(ISR_CHIP_PATH);


    #if 0
    dac.SetDutyCycle(DAC_IN1, 0);
    dac.SetDutyCycle(DAC_IN2, DAC_PCA_MAX_DUTY_CYCLE);
    dac.SetDutyCycle(DAC_SPEED, DAC_PCA_MAX_DUTY_CYCLE);

    // dac.SetDutyCycle(5, 0);
    // dac.SetDutyCycle(6, DAC_PCA_MAX_DUTY_CYCLE);
    // dac.SetDutyCycle(7, DAC_PCA_MAX_DUTY_CYCLE);

    // dac.SetDutyCycle(10, 0);
    // dac.SetDutyCycle(9, DAC_PCA_MAX_DUTY_CYCLE);
    // dac.SetDutyCycle(8, DAC_PCA_MAX_DUTY_CYCLE);

    // dac.SetDutyCycle(11, 0);
    // dac.SetDutyCycle(12, DAC_PCA_MAX_DUTY_CYCLE);
    // dac.SetDutyCycle(13, DAC_PCA_MAX_DUTY_CYCLE);


    // dac.SetDutyCycle(15, DAC_PCA_MAX_DUTY_CYCLE);
    // dac.SetDutyCycle(14, DAC_PCA_MAX_DUTY_CYCLE);
    // dac.SetDutyCycle(0 , DAC_PCA_MAX_DUTY_CYCLE);
    // dac.SetDutyCycle(1 , DAC_PCA_MAX_DUTY_CYCLE);
    dac.UpdateRegisters();

    dac.FlushQueues();

    usleep(10e6);

    #else
    Driver driver = Driver( &dac, DAC_IN1, DAC_IN2, DAC_SPEED,
                            &isr, ENC_A, ENC_B,
                            NULL, ADC_CHANNEL );

    driver.SetVelocityFunc(&velocity_sine);
    driver.SetSpeedCoefficient(100);

    isr.AllocatePins();
    dac.InitDevice();


    struct timeval start, stop;
    while(1)
    {
        gettimeofday(&start, NULL);
        while(1)
        {
            isr.ProcessEvents();    // Check GPIO interrupts (could be an abort signal)
            // TODO                 // Check ADC values (overcurrent / overexertion)
            driver.Poll();          // Update motors with new control information
            dac.FlushQueues();      // Flush pending DAC writes

            gettimeofday(&stop, NULL);
            if (stop.tv_sec * 10e6+ stop.tv_usec  >= 3e6) break;
            usleep(2.5e3);  // 25 ms delay (defacto delay in Talos Operator so far)
        }
    }
    #endif



    /*************************************************************/
    /*************************END SANDBOX*************************/
    /*************************************************************/

    // End demo
    LOG_INFO("End Program.");
    LOG_stop();
    LOG_destory();
}
