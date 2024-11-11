#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stddef.h>

#include "acl/acl.h"
#include "acl/acl_private.h"
#include "util/comm.h"
#include "log/log.h"

#define LOG_CONSOLE_THRESHOLD_THIS  LOG_THRESHOLD_DEFAULT
#define LOG_FILE_THRESHOLD_THIS     LOG_THRESHOLD_MAX

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

int ACL_enqueue_manual_mode_toggle_cmd(S_List *cmd_queue)
{
    S_List_Node* node = DATA_S_List_pop(&resources.free_queue);
    if (!node) STD_FAIL;

    ACL_Command* cmd = DATA_LIST_GET_OBJ(node, ACL_Command, node);
    cmd->type = ACL_CMD_MANUAL;
    cmd->len = sprintf(&cmd->payload[0], ACL_TOGGLE_MANUAL_FMT);

    DATA_S_List_append(cmd_queue, &cmd->node);
    return 0;
}

char ACL_get_polar_pan_continuous_vector(API_Data_Polar_Pan_Start* payload)
{
    ACL_Manual_Axis vector;

    if (payload->delta_azimuth)
        vector = payload->delta_azimuth > 0 ? ACL_MAN_POS_BASE_AXIS : ACL_MAN_NEG_BASE_AXIS;

    else if (payload->delta_altitude)
        vector = payload->delta_altitude > 0 ? ACL_MAN_POS_WRIST_PITCH_AXIS : ACL_MAN_NEG_WRIST_PITCH_AXIS;

    else
    {
        LOG_IEC();
        return '\0';
    }

    return (char) vector;
}

int ACL_calc_enqueue_shift_cmd(S_List *cmd_queue, ACL_Axis axis, float degree_count)
{
    S_List_Node* node = DATA_S_List_pop(&resources.free_queue);
    if (!node) STD_FAIL;

    ACL_Command* cmd = DATA_LIST_GET_OBJ(node, ACL_Command, node);

    int encoder_count;

    float conversion_factor;

    switch (axis)
    {
        case ACL_AXIS_BASE:
            conversion_factor = ACL_BASE_CONVERSION_FACTOR;
            break;

        case ACL_AXIS_WRIST_PITCH:
            conversion_factor = ACL_WRIST_CONVERSION_FACTOR;
            break;

        default:
            LOG_WARN("Degree -> Encoder Count Conversion Failed.");
            STD_FAIL;
    }

    encoder_count = (int)(degree_count * conversion_factor);

    cmd->len = sprintf(&cmd->payload[0], ACL_SHIFT_FMT, ACL_VAR_POS, axis, encoder_count);
    cmd->type = ACL_CMD_SHIFT;

    DATA_S_List_append(cmd_queue, &cmd->node);

    return 0;
}

int ACL_generate_enqueue_here_cmd(S_List *cmd_queue)
{
    S_List_Node* node = DATA_S_List_pop(&resources.free_queue);
    if (!node) STD_FAIL;

    ACL_Command* cmd = DATA_LIST_GET_OBJ(node, ACL_Command, node);

    cmd->len = sprintf(&cmd->payload[0], ACL_HERE_FMT, ACL_VAR_POS);
    cmd->type = ACL_CMD_HERE;

    DATA_S_List_append(cmd_queue, &cmd->node);

    return 0;
}

int ACL_generate_enqueue_move_cmd(S_List *cmd_queue)
{
    S_List_Node* node = DATA_S_List_pop(&resources.free_queue);
    if (!node) STD_FAIL;

    ACL_Command* cmd = DATA_LIST_GET_OBJ(node, ACL_Command, node);

    cmd->len = sprintf(&cmd->payload[0], ACL_MOVE_FMT, ACL_VAR_POS);
    cmd->type = ACL_CMD_MOVE;

    DATA_S_List_append(cmd_queue, &cmd->node);

    return 0;
}

int ACL_generate_enqueue_home_cmd(S_List *cmd_queue)
{
    S_List_Node* node = DATA_S_List_pop(&resources.free_queue);
    if (!node) STD_FAIL;

    ACL_Command* cmd = DATA_LIST_GET_OBJ(node, ACL_Command, node);

    cmd->len = sprintf(&cmd->payload[0], ACL_HOME_FMT);
    cmd->type = ACL_CMD_HOME;

    DATA_S_List_append(cmd_queue, &cmd->node);

    return 0;
}

int ACL_generate_enqueue_defp_cmd(S_List *cmd_queue)
{
    S_List_Node* node = DATA_S_List_pop(&resources.free_queue);
    if (!node) STD_FAIL;

    ACL_Command* cmd = DATA_LIST_GET_OBJ(node, ACL_Command, node);

    cmd->len = sprintf(&cmd->payload[0], ACL_DEFP_FMT, ACL_VAR_POS);
    cmd->type = ACL_CMD_DEFP;

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
        ACL_calc_enqueue_shift_cmd(cmd_queue, ACL_AXIS_WRIST_PITCH, pan->delta_altitude);
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