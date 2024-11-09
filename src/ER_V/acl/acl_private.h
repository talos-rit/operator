#include "acl/acl.h"
#define CMD_SIZE 32

#define VAR_POS "DELTA" /** variable position moniker for ACL commands */

/** Conversion factors for Encoder Count */
#define BASE_CONVERSION_FACTOR 42.5666f
#define SHOULDER_CONVERSION_FACTOR 33.2121f
#define WRIST_CONVERSION_FACTOR 8.3555f

#define ACL_CMD_COUNT 10

/** ACL Command Formats */
#define ACL_SHIFT   "SHIFT %s BY %u %d\r" // Shift command to move a variable the Scorbot has stored
#define ACL_HERE    "HERE %s\r" // Here command to set the input Scorbot position to its current position
#define ACL_MOVE    "MOVE %s\r" // Move command to move to a set point
#define ACL_HOME    "HOME\r" // Home command homes the robot
#define ACL_DEFP    "DEFP %s\r" // Defp command sets an internal variable to the current position for Scorbot
#define ACL_TOGGLE_MANUAL   "~" // While the scorbot controller is in direct mode, this command allows manual control of the position (like a joystick)
#define ACL_MOVE_MANUAL     "%c"

// /** ACL Command Types */
// typedef enum _acl_command_type
// {
//     ACL_CMD_HOME,
//     ACL_CMD_MOVE,
//     ACL_CMD_SHIFT,
//     ACL_CMD_DEFP,
//     ACL_CMD_HERE
// } ACL_Command_Type;

/** Scorbot axes representation in ACL */
typedef enum _acl_axis
{
    ACL_AXIS_BASE                   = 1,
    ACL_AXIS_SHOULDER               = 2,
    ACL_AXIS_ELBOW                  = 3,
    ACL_AXIS_WRIST_PITCH            = 4,
    ACL_AXIS_WRIST_ROLL             = 5
} ACL_Axis;

typedef enum _acl_manual_axis
{
    ACL_MAN_POS_BASE_AXIS           = '1',
    ACL_MAN_NEG_BASE_AXIS           = 'q',
    ACL_MAN_POS_SHOULDER_AXIS       = '2',
    ACL_MAN_NEG_SHOULDER_AXIS       = 'w',
    ACL_MAN_POS_ELBOW_AXIS          = '3',
    ACL_MAN_NEG_ELBOW_AXIS          = 'e',
    ACL_MAN_POS_WRIST_PITCH_AXIS    = '4',
    ACL_MAN_NEG_WRIST_PITCH_AXIS    = 'r',
    ACL_MAN_POS_WRIST_ROLL_AXIS     = '5',
    ACL_MAN_NEG_WRIST_ROLL_AXIS     = 't',
} ACL_Manual_Axis;

/** Resource management struct for ACL commands and the associated S_List */
typedef struct _acl_resources
{
    ACL_Command cmd_pool[ACL_CMD_COUNT];
    S_List      free_queue;
    uint8_t     manaul_mode;
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

