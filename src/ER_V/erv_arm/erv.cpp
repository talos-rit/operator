#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <err.h>
#include <errno.h>
#include <stddef.h>

#include "erv_arm/erv.h"
#include "log/log.h"
#include "acl/acl.h"

#define LOG_CONSOLE_THRESHOLD_THIS  LOG_THRESHOLD_MAX
#define LOG_FILE_THRESHOLD_THIS     LOG_THRESHOLD_MAX

#define ERV_DEFAULT_COMMAND_DELAY 200000

Scorbot::Scorbot(const char* dev)
{
    strcpy(&this->dev[0], &dev[0]);
    fd = open(dev, O_RDWR | O_NOCTTY | O_NDELAY);
    if (fd < 0) LOG_ERROR("Could not open file: %s", strerror(errno));
    ACL_init();
}

Scorbot::~Scorbot()
{
    if(close(fd)) LOG_ERROR("Scorbot: Could not close device descriptor: %s", strerror(errno));
}

void Scorbot::Poll()
{
    const uint8_t size = 255;
    static uint8_t len = 0;
    uint8_t start = len;
    static char buffer[size];
    // LOG_INFO("LEN: %d", len);
    memset(&buffer[0], 0xFF, size);
    len += read(fd, &buffer[0], size);
    // LOG_INFO("LEN: %d", len);

    for (uint8_t iter = start; iter < len && iter < size; iter++)
    {
        if ('\n' == buffer[iter] || '\0' == buffer[iter])
        {
            buffer[iter] = '\0';
            LOG_INFO("SCOR: %s", buffer);
            strcpy(&buffer[0], &buffer[start]);
            len = iter - start;
        }
    }
}

int Scorbot::HandShake()
{
    LOG_INFO("Scorbot Recevied Home Command");
    return 0;
}

int Scorbot::PolarPan(API_Data_Polar_Pan *pan)
{
    uint8_t iter = 0;
    char text[255];

    S_List cmd_list;
    DATA_S_List_init(&cmd_list);
    ACL_convert_polar_pan(&cmd_list, pan);

    ACL_Command* command;

    while (cmd_list.len)
    {
        command = DATA_LIST_GET_OBJ(DATA_S_List_pop(&cmd_list), ACL_Command, node);
        write(fd, &command->payload[0], command->len);
        LOG_VERBOSE(4, "Sending Command: %s", &command->payload[0]);
        usleep(ERV_DEFAULT_COMMAND_DELAY);
        ACL_Command_init(command);
    }


    iter += sprintf(&text[iter], "Scorbot Received Polar Pan Command:\n");
    iter += sprintf(&text[iter], "\tΔ Azimuth: \t%d\n",     pan->delta_azimuth);
    iter += sprintf(&text[iter], "\tΔ Altitude: \t%d\n",    pan->delta_altitude);
    iter += sprintf(&text[iter], "\tDelay: \t\t%d\n",       pan->delay_ms);
    iter += sprintf(&text[iter], "\tTime: \t\t%d\n",        pan->time_ms);

    LOG_INFO("%s", text);

    return 0;
}

int Scorbot::Home(API_Data_Home* home)
{
    uint8_t iter = 0;
    char text[255];

    iter += sprintf(&text[iter], "Scorbot Received Home Command:\n");
    iter += sprintf(&text[iter], "\tDelay: \t\t%d\n",       home->delay_ms);

    LOG_INFO("%s", text);
    write(fd, "home\r", 5);
    write(fd, "defp delta\r", 11); // defines position delta variable
    return 0;
}