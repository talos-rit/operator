#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stddef.h>

#include "acl/acl.h"
#include "acl/acl_private.h"
#include "util/comm.h"
#include "log/log.h"

#define LOG_CONSOLE_THRESHOLD_THIS  LOG_THRESHOLD_MAX
#define LOG_FILE_THRESHOLD_THIS     LOG_THRESHOLD_MAX

/** (Bytes_s * delay_us) / (10^6) */
#define ACL_BYTES_PER_DELAY ((ACL_DEFAULT_COMMAND_DELAY_USEC) * (ACL_DEV_BYTES_PER_SEC) / (1000 * 1000))


ACL_Resources resources;

int ACL_Command_init(ACL_Command *cmd)
{
    if (!cmd) STD_FAIL;

    memset(cmd, 0, sizeof(ACL_Command));

    DATA_S_List_Node_init(&cmd->node);
    DATA_S_List_append(&resources.free_queue, &cmd->node);

    return 1;
}

int ACL_init()
{
    memset(&resources, 0, sizeof(resources));
    for(uint16_t iter = 0; iter < ACL_CMD_COUNT; iter++)
    {
        ACL_Command_init(&resources.cmd_pool[iter]);
    }

    resources.manaul_mode = 0;  // Default is not manual mode
    return 0;
}

int ACL_generate_enqueue_manual_mode_cmd(S_List *cmd_queue)
{
    ACL_Command* cmd = DATA_LIST_GET_OBJ(DATA_S_List_pop(&resources.free_queue), ACL_Command, node);

    if (!cmd) STD_FAIL;

    cmd->len = sprintf(&cmd->payload[0], ACL_TOGGLE_MANUAL);

    DATA_S_List_append(cmd_queue, &cmd->node);

    return 0;
}

int ACL_generate_enqueue_polar_pan_start_cmd(S_List *cmd_queue, int8_t delta_azimuth, int8_t delta_altitude)
{
    S_List_Node* node = DATA_S_List_pop(&resources.free_queue);
    if (!node) STD_FAIL;

    ACL_Command* cmd = DATA_LIST_GET_OBJ(node, ACL_Command, node);
    ACL_Manual_Axis vector;
    cmd->len = 0;

    while (cmd->len < ACL_BYTES_PER_DELAY)
    {
        if (delta_azimuth)
        {
            vector = delta_azimuth > 0 ? ACL_MAN_POS_BASE_AXIS : ACL_MAN_NEG_BASE_AXIS;
            cmd->len += sprintf(&cmd->payload[cmd->len], ACL_MOVE_MANUAL, vector);
        }

        if (delta_azimuth)
        {
            vector = delta_altitude > 0 ? ACL_MAN_POS_BASE_AXIS : ACL_MAN_NEG_BASE_AXIS;
            cmd->len += sprintf(&cmd->payload[cmd->len], ACL_MOVE_MANUAL, vector);
        }
    }

    DATA_S_List_append(cmd_queue, &cmd->node);
    return 0;
}

int ACL_calc_enqueue_shift_cmd(S_List *cmd_queue, ACL_Axis axis, float degree_count)
{
    ACL_Command* cmd = DATA_LIST_GET_OBJ(DATA_S_List_pop(&resources.free_queue), ACL_Command, node);

    if (!cmd) STD_FAIL;

    int encoder_count;

    float conversion_factor;

    switch (axis)
    {
        case ACL_AXIS_BASE:
            conversion_factor = BASE_CONVERSION_FACTOR;
            break;

        case ACL_AXIS_SHOULDER:
            conversion_factor = SHOULDER_CONVERSION_FACTOR;
            break;

        default:
            LOG_WARN("Degree -> Encoder Count Conversion Failed.");
            STD_FAIL;
    }

    encoder_count = (int)(degree_count * conversion_factor);

    cmd->len = sprintf(&cmd->payload[0], ACL_SHIFT, VAR_POS, axis, encoder_count);

    DATA_S_List_append(cmd_queue, &cmd->node);

    return 0;
}

int ACL_generate_enqueue_here_cmd(S_List *cmd_queue)
{
    ACL_Command* cmd = DATA_LIST_GET_OBJ(DATA_S_List_pop(&resources.free_queue), ACL_Command, node);

    if (!cmd) STD_FAIL;

    cmd->len = sprintf(&cmd->payload[0], ACL_HERE, VAR_POS);

    DATA_S_List_append(cmd_queue, &cmd->node);

    return 0;
}

int ACL_generate_enqueue_move_cmd(S_List *cmd_queue)
{
    ACL_Command* cmd = DATA_LIST_GET_OBJ(DATA_S_List_pop(&resources.free_queue), ACL_Command, node);

    if (!cmd) STD_FAIL;

    cmd->len = sprintf(&cmd->payload[0], ACL_MOVE, VAR_POS);

    DATA_S_List_append(cmd_queue, &cmd->node);

    return 0;
}

int ACL_generate_enqueue_home_cmd(S_List *cmd_queue)
{
    ACL_Command* cmd = DATA_LIST_GET_OBJ(DATA_S_List_pop(&resources.free_queue), ACL_Command, node);

    if (!cmd) STD_FAIL;

    cmd->len = sprintf(&cmd->payload[0], ACL_HOME);

    DATA_S_List_append(cmd_queue, &cmd->node);

    return 0;
}

int ACL_generate_enqueue_defp_cmd(S_List *cmd_queue)
{
    ACL_Command* cmd = DATA_LIST_GET_OBJ(DATA_S_List_pop(&resources.free_queue), ACL_Command, node);

    if (!cmd) STD_FAIL;

    cmd->len = sprintf(&cmd->payload[0], ACL_DEFP, VAR_POS);

    DATA_S_List_append(cmd_queue, &cmd->node);

    return 0;
}


int ACL_convert_polar_pan(S_List *cmd_queue, const API_Data_Polar_Pan *pan)
{
    if (!cmd_queue || !pan) STD_FAIL;

    LOG_VERBOSE(4, "Convering polar pan command to ACL");

    ACL_generate_enqueue_here_cmd(cmd_queue);

    if (pan->delta_azimuth != 0)
    {
        LOG_VERBOSE(4, "Converting Delta Azimuth");
        ACL_calc_enqueue_shift_cmd(cmd_queue, ACL_AXIS_BASE, pan->delta_azimuth);
    }

    if (pan->delta_altitude != 0)
    {
        LOG_VERBOSE(4, "Converting Delta Altitude");
        ACL_calc_enqueue_shift_cmd(cmd_queue, ACL_AXIS_SHOULDER, pan->delta_altitude);
    }

    ACL_generate_enqueue_move_cmd(cmd_queue);

    return 0;
}

int ACL_home_sequence(S_List *cmd_queue)
{
    if (!cmd_queue) STD_FAIL;

    ACL_generate_enqueue_home_cmd(cmd_queue);
    ACL_generate_enqueue_defp_cmd(cmd_queue);

    return 0;
}