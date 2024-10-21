#include <stdio.h>

#include "erv_arm/erv.h"
#include "log/log.h"

#define LOG_CONSOLE_THRESHOLD_THIS  LOG_THRESHOLD_MAX
#define LOG_FILE_THRESHOLD_THIS     LOG_THRESHOLD_MAX

int Scorbot::HandShake()
{
    LOG_INFO("Scorbot Recevied Home Command");
    return 0;
}

int Scorbot::PolarPan(API_Data_Polar_Pan *pan)
{
    uint8_t iter = 0;
    char text[255];

    iter += sprintf(&text[iter], "Scorbot Received Polar Pan Command:\n");
    iter += sprintf(&text[iter], "\tΔ Azimuth: \t%d\n",     pan->delta_azimuth);
    iter += sprintf(&text[iter], "\tΔ Altitude: \t%d\n",    pan->delta_altitude);
    iter += sprintf(&text[iter], "\tDelay: \t\t%d\n",       pan->delay_ms);
    iter += sprintf(&text[iter], "\tTime: \t\t%d\n",        pan->time_ms);

    LOG_INFO("%s", text);
    return 0;
}

int Scorbot::Home()
{
    LOG_INFO("Scorbot Recevied Home Command");
    return 0;
}