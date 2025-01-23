#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <err.h>
#include <errno.h>
#include <stddef.h>
#include <sys/time.h>
#include <termios.h>
#include <signal.h>

#include "log/log.h"

#include "arm/ichor_arm.h"

#define LOG_CONSOLE_THRESHOLD_THIS  LOG_THRESHOLD_DEFAULT
#define LOG_FILE_THRESHOLD_THIS     LOG_THRESHOLD_MAX

#define ERV_DEFAULT_COMMAND_DELAY 200000
#define ERV_TTY_BUFFER_LEN 127

#define ERV_CLOCK CLOCK_REALTIME

Ichor::Ichor()
{
}

Ichor::~Ichor()
{
}

void Ichor::Poll()
{
}

int Ichor::HandShake()
{
    return 0;
}

int Ichor::PolarPan(API_Data_Polar_Pan *pan)
{

    switch(oversteer)
    {
        case OVERSTEER_NONE:
            break;
        case OVERSTEER_IGNORE:
            break;
        case OVERSTEER_ABORT:
            break;
        default:
            STD_FAIL;
    }


    uint8_t iter = 0;
    char text[255];

    iter += sprintf(&text[iter], "Polar Pan Payload:\n");
    iter += sprintf(&text[iter], "\tΔ Azimuth: \t%d\n",     pan->delta_azimuth);
    iter += sprintf(&text[iter], "\tΔ Altitude: \t%d\n",    pan->delta_altitude);
    iter += sprintf(&text[iter], "\tDelay: \t\t%d\n",       pan->delay_ms);
    iter += sprintf(&text[iter], "\tTime: \t\t%d\n",        pan->time_ms);
    LOG_VERBOSE(4, "%s", text);

    return 0;
}

int Ichor::PolarPanStart(API_Data_Polar_Pan_Start *pan)
{
    uint8_t iter = 0;
    char text[255];

    iter += sprintf(&text[iter], "Polar Pan Start Payload:\n");
    iter += sprintf(&text[iter], "\tΔ Azimuth: \t%d\n",     pan->delta_azimuth);
    iter += sprintf(&text[iter], "\tΔ Altitude: \t%d\n",    pan->delta_altitude);
    LOG_VERBOSE(4, "%s", text);

    return 0;
}

int Ichor::PolarPanStop()
{
    return 0;
}

int Ichor::Home(API_Data_Home* home)
{
    uint8_t iter = 0;
    char text[255];

    iter += sprintf(&text[iter], "Home Payload:\n");
    iter += sprintf(&text[iter], "\tDelay: \t\t%d",       home->delay_ms);

    return 0;
}