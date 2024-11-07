#include "acl/acl.h"
#define CMD_SIZE 32

#define VAR_POS "DELTA" /** variable position moniker for ACL commands */

/** Conversion factors for Encoder Count */
#define BASE_CONVERSION_FACTOR 42.5666f
#define SHOULDER_CONVERSION_FACTOR 33.2121f
#define WRIST_CONVERSION_FACTOR 8.3555f

#define ACL_CMD_COUNT 10

/** ACL Command Formats */
#define SHIFT "SHIFT %s BY %u %d\r" // Shift command to move a variable the Scorbot has stored
#define HERE "HERE %s\r" // Here command to set the input Scorbot position to its current position
#define MOVE "MOVE %s\r" // Move command to move to a set point
#define HOME "HOME\r" // Home command homes the robot
#define DEFP "DEFP %s\r" // Defp command sets an internal variable to the current position for Scorbot

/** ACL Command Types */
typedef enum _acl_command_type
{
    CMD_HOME,
    CMD_MOVE,
    CMD_SHIFT,
    CMD_DEFP,
    CMD_HERE
} ACL_Command_Type;

/** Scorbot axes representation in ACL */
typedef enum _acl_axis
{
    BASE_AXIS = 1,
    SHOULDER_AXIS = 2,
    ELBOW_AXIS = 3,
    WRIST_PITCH_AXIS = 4,
    WRIST_ROLL_AXIS = 5
} ACL_Axis;

/** Resource management struct for ACL commands and the associated S_List */
typedef struct _acl_resources
{
    ACL_Command cmd_pool[ACL_CMD_COUNT];
    S_List      free_queue;
} ACL_Resources;

/**
 * @brief Calculates encoder count and stores the proper ACL shift command payload
 * @details Compares axes and stores the associated ACL command in the input ACL_Command struct
 * @param cmd_queue S_List to fill with commands
 * @param axis Axis being manipulated on Scorbot
 * @param degree_count Amount of degrees to convert to Encoder Count
 * @returns 0 on success, -1 on failure
 */
int ACL_calc_enqueue_shift_cmd(S_List *cmd_queue, ACL_Axis axis, float degree_count);

/**
 * @brief Allocates and enqueues ACL HOME Command in input cmd_queue
 * @param cmd_queue S_List to append to
 * @returns 0 on success, -1 on failure
 */
int ACL_generate_enqueue_here_cmd(S_List *cmd_queue);

/**
 * @brief Allocates and enqueues ACL MOVE Command in input cmd_queue
 * @param cmd_queue S_List to append to
 * @returns 0 on success, -1 on failure
 */
int ACL_generate_enqueue_move_cmd(S_List *cmd_queue);

