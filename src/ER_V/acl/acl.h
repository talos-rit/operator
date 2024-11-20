/**
 * ACL Conversion Module
 *
 * This module is responsible for the conversion of received movement commands to ACL
 * ACL is the programming language we use to communicate with the Scorbot-ER V
 */

#pragma once

#include <stdint.h>

#include "api/api.h"
#include "data/s_list.h"

#define ACL_CMD_SIZE 64

#define ACL_DEFAULT_COMMAND_DELAY_MS 200
#define ACL_DEV_BAUD 9600
#define ACL_DEV_FRAME_SIZE 10
#define ACL_DEV_BYTES_PER_SEC (ACL_DEV_BAUD / ACL_DEV_FRAME_SIZE)

/** (Bytes_s * delay_us) / (10^6) */
// #define ACL_BYTES_PER_DELAY ((ACL_DEFAULT_COMMAND_DELAY_MS) * (ACL_DEV_BYTES_PER_SEC) / (1000 * 1000))
#define ACL_MANUAL_MOVE_SIZE 1

/** ACL Command Types */
typedef enum _acl_command_type
{
    ACL_CMD_INVALID,
    ACL_CMD_DELAY,
    ACL_CMD_HOME,
    ACL_CMD_MOVE,
    ACL_CMD_SHIFT,
    ACL_CMD_DEFP,
    ACL_CMD_HERE,
    ACL_CMD_MANUAL,
    ACL_CMD_ABORT,
} ACL_Command_Type;

/** ACL Command Map for the command string, length, and node */
typedef struct _acl_command
{
  char              payload[ACL_CMD_SIZE];    /** The ACL command (dynamically sized)*/
  uint8_t           len;                      /** The length of the ACL command */
  ACL_Command_Type  type;                     /** ACL command type */
  uint16_t          delay_ms;                 /** Milliseconds between this command and the next command */
  S_List_Node       node;                     /** The node of the s_list */
} ACL_Command;

/**
 * @brief Allocates memory for an ACL_Resources' array of ACL_Commands
 * @details Iterates through command pool in global ACL_Resources and populates with dummy data
  * @returns 0 on success, -1 on failure
 */
int ACL_init();

/**
 * @brief Allocates memory for a single ACL_Command and initializes its S_List node
 * @details Fills the input cmd with dummy data in the form of 0's
 * Initializes cmd's S_List_Node; appends node to global resource's S_List
 * @param cmd ACL_Command to initialize
  * @returns 0 on success, -1 on failure
 */
int ACL_Command_init(ACL_Command *cmd);

/**
 * @brief Populates cmd_queue with a formatted queue for executing a polar pan, according to contents of pan. Aborts last movement (not cummulative).
 * @details Polar Pan command struct is processed and turned into a set of ACL commands
 * ACL Commands are stored in the input empty S_List
 * @param cmd_queue S_List pointer to be manipulated into a queue of ACL Commands
 * @param pan Polar Pan command struct pointer
 * @returns 0 on success, -1 on failure
 */
int ACL_convert_polar_pan_abort(S_List *cmd_queue, const API_Data_Polar_Pan *pan);

/**
 * @brief Populates cmd_queue with a formatted queue for executing a polar pan, according to contents of pan. Accumlates onto last movement command
 * @details Polar Pan command struct is processed and turned into a set of ACL commands
 * ACL Commands are stored in the input empty S_List
 * @param cmd_queue S_List pointer to be manipulated into a queue of ACL Commands
 * @param pan Polar Pan command struct pointer
 * @returns 0 on success, -1 on failure
 */
int ACL_convert_polar_pan_direct(S_List *cmd_queue, const API_Data_Polar_Pan *pan);

/**
 * @brief Populates cmd_queue with a formatted queue for executing a polar pan, according to contents of pan. Drops movements while the current one is finishing.
 * @details Polar Pan command struct is processed and turned into a set of ACL commands
 * ACL Commands are stored in the input empty S_List
 * @param cmd_queue S_List pointer to be manipulated into a queue of ACL Commands
 * @param pan Polar Pan command struct pointer
 * @returns 0 on success, -1 on failure
 */
int ACL_convert_polar_pan_ignore(S_List *cmd_queue, const API_Data_Polar_Pan *pan);

/**
 * @brief Populates cmd_queue with a formatted queue for toggle manual mode on the controller unit
 * @details Only works when the Scorbot ER V controller is in DIRECT mode
 * @returns 0 on success, -1 on failure
*/
int ACL_enqueue_manual_mode_toggle_cmd(S_List *cmd_queue);

/**
 * @brief Allocates and enqueues ACL HOME Command in input cmd_queue
 * @param cmd_queue S_List to append to
 * @returns 0 on success, -1 on failure
 */
int ACL_enqueue_here_cmd(S_List *cmd_queue);

/**
 * @brief Inserts a delay into the cmd_queue sequence
 * @param cmd_queue S_List to append to
 * @param delay_ms Number of milliseconds to delay
 * @returns 0 on success, -1 on failure
 */
int ACL_enqueue_delay(S_List *cmd_queue, uint16_t delay_ms);

/**
 * @brief Generates and enqueues a home command and sets ACL_VAR_POS as the movable variable
 * @param cmd_queue The command queue to fill
 * @returns 0 on success, -1 on failure
 */
int ACL_home_sequence(S_List *cmd_queue);

/**
 * @brief Returns the ACL character corresponding to the desired angular vector, according to the received payload
 * @returns Corresponding ACL character on success, NULL character on failure
*/
char ACL_get_polar_pan_continuous_vector(API_Data_Polar_Pan_Start* payload);
