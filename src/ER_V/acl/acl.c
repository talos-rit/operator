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

int ACL_Command_init(ACL_Command *cmd)
{
    if (!cmd) STD_FAIL;

    memset(cmd, 0, sizeof(ACL_Command));
    
    DATA_S_List_Node_init(&cmd->node);
    DATA_S_List_append(&resources.queue, &cmd->node);

    return 1;
}

int ACL_init()
{
    for(uint16_t iter = 0; iter < ACL_CMD_COUNT; iter++)
    {
        ACL_Command_init(&resources.cmd_pool[iter]);
    }

    return 1;
}

int ACL_calc_enqueue_shift_command(ACL_Command *cmd, ACL_Axis axis, float degree_count)
{
    if (!cmd) STD_FAIL;

    int encoder_count;

    if (axis == BASE_AXIS)
    {
        encoder_count = (int)(degree_count * BASE_CONVERSION_FACTOR);
    }
    else if (axis == SHOULDER_AXIS)
    {
        encoder_count = (int)(degree_count * SHOULDER_CONVERSION_FACTOR);
    }

    cmd->len = sprintf(&cmd->payload[0], SHIFT, VAR_POS, axis, encoder_count);
    return 1;
}

int ACL_command_generate_enqueue_movement(S_List *cmd_queue, ACL_Command_Type cmd_type, ACL_Axis axis, float degree_count)
{
    ACL_Command* cmd = DATA_LIST_GET_OBJ(DATA_S_List_pop(&resources.queue), ACL_Command, node);

    switch (cmd_type)
    {
        case CMD_SHIFT:
            ACL_calc_enqueue_shift_command(cmd, axis, degree_count);
            break;

        case CMD_HERE:
            cmd->len = sprintf(&cmd->payload[0], HERE, VAR_POS);
            break;

        case CMD_MOVE:
            cmd->len = sprintf(&cmd->payload[0], MOVE, VAR_POS);
            break;
        default:
            LOG_INFO("Unrecognized Command while enqueing");
            STD_FAIL;
    }

    DATA_S_List_append(cmd_queue, &cmd->node);
    return 1;
}

/** @brief readability function for non-movement commands */
static void ACL_command_generate_enqueue_generic(S_List *cmd_queue, ACL_Command_Type cmd_type)
{
    ACL_command_generate_enqueue_movement(cmd_queue, cmd_type, BASE_AXIS, 0.0);
}

int ACL_convert_polar_pan(S_List *cmd_queue, const API_Data_Polar_Pan *pan)
{
    if (!cmd_queue || !pan) STD_FAIL;

    LOG_INFO("Convering polar pan command to ACL");

    ACL_command_generate_enqueue_generic(cmd_queue, CMD_HERE); // HERE command

    if (pan->delta_azimuth != 0)
    {
        LOG_INFO("Converting Delta Azimuth");
        ACL_command_generate_enqueue_movement(cmd_queue, CMD_SHIFT, BASE_AXIS, pan->delta_azimuth); // SHIFT BY 1 (Base) Command
    }

    if (pan->delta_altitude != 0)
    {
        LOG_INFO("Converting Delta Altitude");
        ACL_command_generate_enqueue_movement(cmd_queue, CMD_SHIFT, SHOULDER_AXIS, pan->delta_altitude); // SHIFT BY 2 (Shoulder) Command
    }

    ACL_command_generate_enqueue_generic(cmd_queue, CMD_MOVE); // MOVE Command

    return 1;
}