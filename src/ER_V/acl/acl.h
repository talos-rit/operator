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


/** Struct mapping ACL commands as an s_list for proper queing of commands */
typedef struct _acl_command
{
  char        payload[ACL_SIZE]; /** The ACL command (dynamically sized)*/
  uint8_t     len;       /** The length of the ACL command */
  S_List_Node node;      /** The node of the s_list */
} ACL_Command;

int ACL_init();

int ACL_Command_init(ACL_Command *cmd);

/**
 * @brief Populates s_list with formatted queue, according to contents of pan
 * @details Polar Pan command struct is processed and turned into a set of ACL commands
 * ACL Commands are stored in the input empty S_List
 * @param cmd_queue S_List pointer to be manipulated into a queue of ACL Commands
 * @param pan Polar Pan command struct pointer
 * @returns 0 if successful, -1 if not
 */
int ACL_convert_polar_pan(S_List *cmd_queue, const API_Data_Polar_Pan *pan);

/**
 * @brief Populates s_list with home command
 */
int ACL_home_command(S_List *cmd_queue);
