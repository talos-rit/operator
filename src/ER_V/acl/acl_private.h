#include "acl/acl.h"
#define CMD_SIZE 32

#define VAR_POS "DELTA" /** variable position moniker for ACL commands */

/** Conversion factors for Encoder Count */
#define BASE_CONVERSION_FACTOR 42.5666f
#define SHOULDER_CONVERSION_FACTOR 33.2121f
#define WRIST_CONVERSION_FACTOR 8.3555f

#define ACL_CMD_COUNT 10

/** ACL Command Formats */
#define SHIFT "SHIFT %s BY %u %d\r"
#define HERE "HERE %s"
#define MOVE "MOVE %s\r"

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
    S_List      queue;
} ACL_Resources;

/**
 * @brief Calculates encoder count and stores the proper ACL shift command payload
 * @details Compares axes and stores the associated ACL command in the input ACL_Command struct
 * @param cmd ACL_Command to be filled
 * @param axis Axis being manipulated on Scorbot
 * @param degree_count Amount of degrees to convert to Encoder Count
 * @returns 1 if successful
 */
int ACL_calc_enqueue_shift_command(ACL_Command *cmd, ACL_Axis axis, float degree_count);

/**
 * @brief Generates ACL_Command payload based on input axis and cmd_type and enqueues it in the input cmd_queue
 * @details Grabs pre-allocated ACL_Command from global resources; populates payload based on input axis and degree_count
 * Appends ACL_Command's node to cmd_queue
 * @param cmd_queue The S_List to add the generated ACL_Command to
 * @param cmd_type The type of ACL command
 * @param axis Axis being manipulated on Scorbot
 * @param degree_count Amount of degrees to move
 */
int ACL_command_generate_enqueue_movement(S_List *cmd_queue, ACL_Command_Type cmd_type, ACL_Axis axis, float degree_count);