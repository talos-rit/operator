#include "acl/acl.h"

#define ACL_VAR_POS "DELTA" /** variable position moniker for ACL commands */

/** Conversion factors for Encoder Count */
#define ACL_BASE_CONVERSION_FACTOR 42.5666f
#define ACL_SHOULDER_CONVERSION_FACTOR 33.2121f
#define ACL_WRIST_CONVERSION_FACTOR 8.3555f

#define ACL_CMD_COUNT 32

/** ACL Command Formats */
#define ACL_ABORT_FMT           "A\r"   // Aborts the current movment (but keeps the movement buffer in tact)
#define ACL_SHIFT_FMT           "SHIFT %s BY %u %d\r" // Shift command to move a variable the Scorbot has stored
#define ACL_HERE_FMT            "HERE %s\r" // Here command to set the input Scorbot position to its current position
#define ACL_MOVE_FMT            "MOVE %s\r" // Move command to move to a set point
#define ACL_MOVE_DUR_FMT        "MOVE %s %u\r" // Move command to move to a set point, within a specific duration
#define ACL_HOME_FMT            "HOME\r" // Home command homes the robot
#define ACL_DEFP_FMT            "DEFP %s\r" // Defp command sets an internal variable to the current position for Scorbot
#define ACL_TOGGLE_MANUAL_FMT   "~" // While the scorbot controller is in direct mode, this command allows manual control of the position (like a joystick)
#define ACL_MOVE_MANUAL_FMT     "%c"
#define ACL_CLRBUF_FMT          "clrbuf\r"  // Clears the movement buffer, and halts all motor movement

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
 * @brief Allocates and enqueues ACL SETPV Command in input cmd_queue, setting it to the home position
 * @param cmd_queue S_List to append to
 * @returns 0 on success, -1 on failure
 */
int ACL_generate_enqueue_null_pos_cmd(S_List *cmd_queue);

/**
 * @brief Allocates and enqueues ACL MOVE Command in input cmd_queue
 * @param cmd_queue S_List to append to
 * @returns 0 on success, -1 on failure
 */
int ACL_generate_enqueue_move_cmd(S_List *cmd_queue);

/**
 * @brief Allocates and enqueues ACL ABORT Command in input cmd_queue
 * @details Aborts all movement/running programs on the ER V controller
 * @param cmd_queue S_List to append to
 * @returns 0 on success, -1 on failure
*/
int ACL_generate_enqueue_clrbuf_cmd(S_List *cmd_queue);

