#include <stdio.h>
#include <math.h>
#include <string.h>

#include "acl/acl.h"
#include "acl/acl_private.h"
#include "log/log.h"

#define LOG_CONSOLE_THRESHOLD_THIS  LOG_THRESHOLD_MAX
#define LOG_FILE_THRESHOLD_THIS     LOG_THRESHOLD_MAX


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

/**
 * @brief Takes input command string and populates an instance of the ACL_Command struct
 */
ACL_Command ACL_build_command(const char *command)
{
    ACL_Command cmd;
    S_List_Node node;
    DATA_S_List_Node_init(&node);

    size_t len = strlen(command);

    if (len >= CMD_SIZE) LOG_WARN("Command string too long; may be truncated");

    cmd.len = len;
    cmd.node = node;
    sprintf(cmd.payload, "%s", command);

    return cmd;

}

void ACL_build_move_command(char *command)
{
    sprintf(command, "MOVE %s\r", VAR_POS);
}

void ACL_build_shift_command(char *command, ACL_Axis axis, int encoder_count)
{
    sprintf(command, "SHIFT %s BY %u %d\r", VAR_POS, axis, encoder_count);
}

void ACL_build_here_command(char *command)
{
    sprintf(command, "HERE %s", VAR_POS);
}

int ACL_init_command(ACL_Command *cmd)
{
    memset(cmd, 0, sizeof(ACL_Command));
    DATA_S_List_Node_init(&cmd->node);
    return 0;
}

void ACL_command_generate_enqueue_movement(S_List *cmd_queue, ACL_Command_Type cmd_type, float degree_count)
{
    ACL_Command cmd;

    char command[CMD_SIZE] = {0};

    switch (cmd_type)
    {
        int encoder_count;

        case CMD_SHIFT_1:
            encoder_count = ACL_convert_generic(degree_count, BASE_CONVERSION_FACTOR);

            ACL_build_shift_command(command, BASE_AXIS, encoder_count);
            break;

        case CMD_SHIFT_2:
            encoder_count = ACL_convert_generic(degree_count, SHOULDER_CONVERSION_FACTOR);

            ACL_build_shift_command(command, SHOULDER_AXIS, encoder_count);
            break;

        case CMD_HERE:
            ACL_build_here_command(command);
            break;

        case CMD_MOVE:
            ACL_build_move_command(command);
            break;
        default:
            LOG_INFO("Unrecognized Command while enqueing");
            return;
    }

    cmd = ACL_build_command(command);
    
    DATA_S_List_append(cmd_queue, &cmd.node);
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

/*
// Assume some uint8_t* payload of polar pan data
API_Data_Polar_Pan *pan = (API_Data_Polar_Pan*) payload;
S_List cmd_list;
S_List_init(&cmd_list);
ACL_convert_polar_pan(&cmd_list, pan);
*/