#include "acl/acl.h"
#define CMD_SIZE 32

#define VAR_POS "DELTA" /** variable position moniker for ACL commands */

// Conversion factors for Encoder Count
#define BASE_CONVERSION_FACTOR 42.5666f
#define SHOULDER_CONVERSION_FACTOR 33.2121f
#define WRIST_CONVERSION_FACTOR 8.3555f

#define ACL_CMD_COUNT 10

typedef enum _acl_command_type
{
    CMD_HOME,
    CMD_MOVE,
    CMD_SHIFT_1, // shift by axis noted (1, 2, 3, 4, 5)
    CMD_SHIFT_2,
    CMD_SHIFT_3,
    CMD_SHIFT_4,
    CMD_SHIFT_5,
    CMD_DEFP,
    CMD_HERE
} ACL_Command_Type;

typedef enum _acl_axis
{
    BASE_AXIS = 1,
    SHOULDER_AXIS = 2,
    ELBOW_AXIS = 3,
    WRIST_PITCH_AXIS = 4,
    WRIST_ROLL_AXIS = 5
} ACL_Axis;

typedef struct _acl_resources
{
    ACL_Command cmd_pool[ACL_CMD_COUNT];
    S_List      queue;
} ACL_Resources;

int ACL_init_command(ACL_Command *cmd);