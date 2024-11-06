#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stddef.h>

#include "acl/acl.h"
#include "acl/acl_private.h"
#include "log/log.h"

#define LOG_CONSOLE_THRESHOLD_THIS  LOG_THRESHOLD_MAX
#define LOG_FILE_THRESHOLD_THIS     LOG_THRESHOLD_MAX


ACL_Resources resources;

/**
 * @brief Calculates encoder count based on input degrees and conversion factor
 * @details does the simple math to conver degrees to encoder count based on input parameters
 * @param degrees Degrees to convert
 * @param encoderConversionFactor Conversion factor to base off of
 * @returns Degree measurement in ambiguous encoder count
 */
int ACL_convert_generic(float degrees, float encoderConversionFactor)
{
    float encoderCount = degrees * encoderConversionFactor;

    return (int)(encoderCount);
}

int ACL_Command_init(ACL_Command *cmd)
{
    if (!cmd) STD_FAIL;
    memset(cmd, 0, sizeof(ACL_Command));
    DATA_S_List_Node_init(&cmd->node);
    DATA_S_List_append(&resources.queue, &cmd->node);
    return 0;
}

int ACL_init()
{
    for(uint16_t iter = 0; iter < ACL_CMD_COUNT; iter++)
    {
        ACL_Command_init(&resources.cmd_pool[iter]);
    }

    return 0;
}

int ACL_build_move_command(ACL_Command *command)
{
    if (!command) STD_FAIL;
    command->len = sprintf(&command->payload[0], "MOVE %s\r", VAR_POS);
    return 0;
}

int ACL_build_shift_command(ACL_Command *command, ACL_Axis axis, int encoder_count)
{
    if (!command) STD_FAIL;
    command->len = sprintf(&command->payload[0], "SHIFT %s BY %u %d\r", VAR_POS, axis, encoder_count);
    return 0;
}

int ACL_build_here_command(ACL_Command *command)
{
    if (!command) STD_FAIL;
    command->len = sprintf(&command->payload[0], "HERE %s", VAR_POS);
    return 0;
}

void ACL_command_generate_enqueue_movement(S_List *cmd_queue, ACL_Command_Type cmd_type, float degree_count)
{
    ACL_Command* cmd = DATA_LIST_GET_OBJ(DATA_S_List_pop(&resources.queue), ACL_Command, node);

    switch (cmd_type)
    {
        int encoder_count;

        case CMD_SHIFT_1:
            encoder_count = ACL_convert_generic(degree_count, BASE_CONVERSION_FACTOR);

            ACL_build_shift_command(cmd, BASE_AXIS, encoder_count);
            break;

        case CMD_SHIFT_2:
            encoder_count = ACL_convert_generic(degree_count, SHOULDER_CONVERSION_FACTOR);

            ACL_build_shift_command(cmd, SHOULDER_AXIS, encoder_count);
            break;

        case CMD_HERE:
            ACL_build_here_command(cmd);
            break;

        case CMD_MOVE:
            ACL_build_move_command(cmd);
            break;
        default:
            LOG_INFO("Unrecognized Command while enqueing");
            return;
    }

    DATA_S_List_append(cmd_queue, &cmd->node);
}

/** @brief readability function for non-movement commands */
void ACL_command_generate_enqueue_generic(S_List *cmd_queue, ACL_Command_Type cmd_type)
{
    ACL_command_generate_enqueue_movement(cmd_queue, cmd_type, 0.0);
}

int ACL_convert_polar_pan(S_List *cmd_queue, const API_Data_Polar_Pan *pan)
{
    LOG_INFO("Convering polar pan command to ACL...");

    ACL_command_generate_enqueue_generic(cmd_queue, CMD_HERE); // HERE command

    if (pan->delta_azimuth != 0)
    {
        LOG_INFO("Converting Delta Azimuth...");
        ACL_command_generate_enqueue_movement(cmd_queue, CMD_SHIFT_1, pan->delta_azimuth); // SHIFT BY 1 Command
    }

    if (pan->delta_altitude != 0)
    {
        LOG_INFO("Converting Delta Altitude...");
        ACL_command_generate_enqueue_movement(cmd_queue, CMD_SHIFT_2, pan->delta_altitude); // SHIFT BY 2 Command
    }

    ACL_command_generate_enqueue_generic(cmd_queue, CMD_MOVE); // MOVE Command

    return 1;
}