#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stddef.h>

#include "acl/acl.h"
#include "acl/acl_private.h"
#include "util/comm.h"
#include "log/log.h"
#include "erv_arm/erv.h"

#define LOG_CONSOLE_THRESHOLD_THIS  LOG_THRESHOLD_DEFAULT
#define LOG_FILE_THRESHOLD_THIS     LOG_THRESHOLD_MAX

ACL_Resources resources;

int ACL_Command_init(ACL_Command *cmd)
{
    if (!cmd) STD_FAIL;

    memset(cmd, 0, sizeof(ACL_Command));

    DATA_S_List_Node_init(&cmd->node);
    DATA_S_List_append(&resources.free_queue, &cmd->node);

    cmd->delay_ms = ACL_DEFAULT_COMMAND_DELAY_MS;

    return 1;
}

int ACL_init()
{
    memset(&resources, 0, sizeof(resources));
    resources.cmd_pool = (ACL_Command*) malloc (ACL_CMD_COUNT * sizeof(ACL_Command));
    resources.cmd_count = ACL_CMD_COUNT;

    for(uint16_t iter = 0; iter < ACL_CMD_COUNT; iter++)
    {
        ACL_Command_init(&resources.cmd_pool[iter]);
    }

    resources.manaul_mode = 0;  // Default is not manual mode
    return 0;
}


int ACL_destroy()
{
    DATA_S_List_deinit(&resources.free_queue);
    resources.cmd_count = 0;
    if (resources.cmd_pool)
    {
        free(resources.cmd_pool);
        resources.cmd_pool = NULL;
    }

    return 0;
}

/**
 * @brief Helper function for fetching initialized ACL Command structs
 * @returns ACL Command pointer on success, NULL on failure
*/
static ACL_Command* get_cmd(S_List *cmd_queue)
{
    if (!cmd_queue) STD_FAIL_VOID_PTR;
    S_List_Node* node = DATA_S_List_pop(&resources.free_queue);
    if (!node) STD_FAIL_VOID_PTR;
    return DATA_LIST_GET_OBJ(node, ACL_Command, node);
}

int ACL_flush_tx(S_List *cmd_queue)
{
    ACL_Command* cmd = get_cmd(cmd_queue);
    if(!cmd) STD_FAIL;

    cmd->type = ACL_CMD_FLUSH;
    cmd->len = sprintf(&cmd->payload[0], "\r");
    cmd->delay_ms = 100;

    DATA_S_List_append(cmd_queue, &cmd->node);
    return 0;
}

int ACL_enqueue_clrbuf_cmd(S_List *cmd_queue)
{
    ACL_Command* cmd = get_cmd(cmd_queue);
    if(!cmd) STD_FAIL;

    cmd->type = ACL_CMD_ABORT;
    cmd->len = sprintf(&cmd->payload[0], ACL_CLRBUF_FMT);
    cmd->delay_ms = 100;

    DATA_S_List_append(cmd_queue, &cmd->node);
    return 0;
}

int ACL_enqueue_manual_mode_toggle_cmd(S_List *cmd_queue)
{
    ACL_Command* cmd = get_cmd(cmd_queue);
    if(!cmd) STD_FAIL;

    cmd->type = ACL_CMD_MANUAL;
    cmd->len = sprintf(&cmd->payload[0], ACL_TOGGLE_MANUAL_FMT);

    DATA_S_List_append(cmd_queue, &cmd->node);
    return 0;
}

char ACL_get_polar_pan_continuous_vector(API_Data_Polar_Pan_Start* payload)
{
    if (!payload) STD_FAIL;

    ACL_Manual_Axis vector;
    if (payload->delta_azimuth)
    {
        vector = payload->delta_azimuth > 0 ? ACL_MAN_POS_BASE_AXIS : ACL_MAN_NEG_BASE_AXIS;
    }
    else if (payload->delta_altitude)
    {
        vector = payload->delta_altitude > 0 ? ACL_MAN_POS_WRIST_PITCH_AXIS : ACL_MAN_NEG_WRIST_PITCH_AXIS;
    }
    else
    {
        // LOG_IEC(); // A directionless start should not be issued
        return '\0';
    }

    return (char) vector;
}

int ACL_enqueue_shift_cmd(S_List *cmd_queue, ACL_Axis axis, float degree_count)
{
    ACL_Command* cmd = get_cmd(cmd_queue);
    if(!cmd) STD_FAIL;

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

int ACL_enqueue_here_cmd(S_List *cmd_queue)
{
    ACL_Command* cmd = get_cmd(cmd_queue);
    if(!cmd) STD_FAIL;

    cmd->len = sprintf(&cmd->payload[0], ACL_HERE_FMT, ACL_VAR_POS);
    cmd->type = ACL_CMD_HERE;

    DATA_S_List_append(cmd_queue, &cmd->node);

    return 0;
}

int ACL_enqueue_delay(S_List *cmd_queue, uint16_t delay_ms)
{
    ACL_Command* cmd = get_cmd(cmd_queue);
    if(!cmd) STD_FAIL;

    cmd->len = 0;
    cmd->type = ACL_CMD_DELAY;
    cmd->delay_ms = delay_ms;

    DATA_S_List_append(cmd_queue, &cmd->node);
    return 0;
}

int ACL_enqueue_move_cmd(S_List *cmd_queue)
{
    ACL_Command* cmd = get_cmd(cmd_queue);
    if(!cmd) STD_FAIL;

    cmd->len = sprintf(&cmd->payload[0], ACL_MOVE_FMT, ACL_VAR_POS);
    cmd->type = ACL_CMD_MOVE;

    DATA_S_List_append(cmd_queue, &cmd->node);
    return 0;
}

int ACL_generate_enqueue_moved_cmd(S_List *cmd_queue)
{
    ACL_Command* cmd = get_cmd(cmd_queue);
    if(!cmd) STD_FAIL;

    cmd->len = sprintf(&cmd->payload[0], ACL_MOVED_FMT, ACL_VAR_POS);
    cmd->type = ACL_CMD_MOVE;

    DATA_S_List_append(cmd_queue, &cmd->node);
    return 0;
}

int ACL_generate_enqueue_home_cmd(S_List *cmd_queue)
{
    ACL_Command* cmd = get_cmd(cmd_queue);
    if(!cmd) STD_FAIL;

    cmd->len = sprintf(&cmd->payload[0], ACL_HOME_FMT);
    cmd->type = ACL_CMD_HOME;
    // cmd->delay_ms = ACL_DEFAULT_HOME_DELAY_MS;

    DATA_S_List_append(cmd_queue, &cmd->node);
    return 0;
}

int ACL_generate_enqueue_defp_cmd(S_List *cmd_queue)
{
    ACL_Command* cmd = get_cmd(cmd_queue);
    if(!cmd) STD_FAIL;

    cmd->len = sprintf(&cmd->payload[0], ACL_DEFP_FMT, ACL_VAR_POS);
    cmd->type = ACL_CMD_DEFP;

    DATA_S_List_append(cmd_queue, &cmd->node);
    return 0;
}

/**
 * @brief Helper function for populating a Command Queue with the movements required for a polar pan
 * @param cmd_queue Command Queue to append to
 * @param pan Polar pan parameters
*/
static void polar_pan_body(S_List *cmd_queue, const API_Data_Polar_Pan *pan)
{
    if (pan->delta_azimuth != 0)
    {
        LOG_VERBOSE(4, "Converting Delta Azimuth");
        ACL_enqueue_shift_cmd(cmd_queue, ACL_AXIS_BASE, pan->delta_azimuth);
    }

    if (pan->delta_altitude != 0)
    {
        LOG_VERBOSE(4, "Converting Delta Altitude");
        ACL_enqueue_shift_cmd(cmd_queue, ACL_AXIS_WRIST_PITCH, pan->delta_altitude);
    }
}

int ACL_convert_polar_pan_abort(S_List *cmd_queue, const API_Data_Polar_Pan *pan)
{
    if (!cmd_queue) STD_FAIL;
    if (!pan)       STD_FAIL;

    LOG_VERBOSE(4, "Converting polar pan abort command to ACL");

    ACL_enqueue_here_cmd(cmd_queue);
    polar_pan_body(cmd_queue, pan);
    ACL_enqueue_clrbuf_cmd(cmd_queue);
    ACL_enqueue_move_cmd(cmd_queue);

    return 0;
}

int ACL_convert_polar_pan_direct(S_List *cmd_queue, const API_Data_Polar_Pan *pan)
{
    if (!cmd_queue) STD_FAIL;
    if (!pan)       STD_FAIL;

    LOG_VERBOSE(4, "Converting polar pan direct command to ACL");

    polar_pan_body(cmd_queue, pan);
    ACL_enqueue_move_cmd(cmd_queue);

    return 0;
}

int ACL_convert_polar_pan_ignore(S_List *cmd_queue, const API_Data_Polar_Pan *pan)
{
    if (!cmd_queue) STD_FAIL;
    if (!pan)       STD_FAIL;

    LOG_VERBOSE(4, "Converting polar pan ignore command to ACL");

    polar_pan_body(cmd_queue, pan);
    ACL_enqueue_move_cmd(cmd_queue);

    return 0;
}

int ACL_home_sequence(S_List *cmd_queue)
{
    if (!cmd_queue) STD_FAIL;

    ACL_generate_enqueue_defp_cmd(cmd_queue);
    ACL_generate_enqueue_home_cmd(cmd_queue);
    ACL_enqueue_here_cmd(cmd_queue);

    return 0;
}