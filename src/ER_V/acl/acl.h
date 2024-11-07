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

#define ACL_SIZE 32


/** ACL Command Map for the command string, length, and node */
typedef struct _acl_command
{
  char        payload[ACL_SIZE]; /** The ACL command (dynamically sized)*/
  uint8_t     len;               /** The length of the ACL command */
  S_List_Node node;              /** The node of the s_list */
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
 * @brief Populates cmd_queue with formatted queue, according to contents of pan
 * @details Polar Pan command struct is processed and turned into a set of ACL commands
 * ACL Commands are stored in the input empty S_List
 * @param cmd_queue S_List pointer to be manipulated into a queue of ACL Commands
 * @param pan Polar Pan command struct pointer
 * @returns 0 on success, -1 on failure
 */
int ACL_convert_polar_pan(S_List *cmd_queue, const API_Data_Polar_Pan *pan);

/**
 * @brief Generates and enqueues a home command and sets VAR_POS as the movable variable
 * @param cmd_queue The command queue to fill
 * @returns 0 on success, -1 on failure
 */
int ACL_home_sequence(S_List *cmd_queue);
