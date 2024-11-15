/**
 * Singly Linked List
 * 
 * The operation of this module is a little different than the standard implementation of an SLL. 
 * Typically, each node in a list stores two things: the next node in the list, and a piece of data
 * For this SLL, a node simply stores the pointer of the next node. The node is a member within the object it tracks
 * To get the pointer to a node's respective object, offset arithmatic is used (DATA_LIST_GET_OBJ in list.h)
 * Doing it this way allows an object to be tracked by multiple lists, and allows the list to be completely agnostic to the data/type it tracks
*/

#pragma once

#include <stdint.h>

#include "data/list.h"

/** Singly Linked List Node */
typedef struct _s_list_node
{
    struct _s_list_node *next;  /** Pointer to next node */
} S_List_Node;

/** Singly Linked List */
typedef struct _s_list
{
    S_List_Node *head;          /** Pointer to head of list */
    S_List_Node *tail;          /** Pointer to tail of list */
    uint32_t len;               /** Length of list */
} S_List;

/**
 * @brief Initializes a Node for a Singly Linked List
 * @param node Node to initialize
 * @returns 0 on success, -1 on failure
*/
int8_t DATA_S_List_Node_init (S_List_Node *node);

/**
 * @brief Initializes a Singly Linked List
 * @param list List to initialize
 * @returns 0 on success, -1 on failure
*/
int8_t DATA_S_List_init (S_List *list);

/**
 * @brief Denitializes a Singly Linked List
 * @param list List to deinitialize
 * @returns 0 on success, -1 on failure
*/
int8_t DATA_S_List_deinit (S_List *list);

/**
 * @brief Append node to the end of a Singly Linked List
 * @param node Node to append
 * @returns 0 on success, -1 on failure
*/
int8_t DATA_S_List_append (S_List *list, S_List_Node *node);

/**
 * @brief Appends the first element of the child list to the last element of the parent list
 * @details Reinitalizes child list
 * @param parent Parent S_List to append elements onto
 * @param child Child S_List to move elements from
 * @returns 0 on success, -1 on failure
*/
int8_t DATA_S_List_append_list(S_List *parent, S_List *child);

/**
 * @brief Inserts node into a Singly Linked List at a specific index
 * @param node Node to insert
 * @param index Index to insert node at
 * @returns 0 on success, -1 on failure
*/
int8_t DATA_S_List_insert (S_List *list, S_List_Node *node, uint16_t index);

/**
 * @brief Prepends node to the beginning of a Singly Linked List
 * @param node Node to prepend
 * @returns 0 on success, -1 on failure
*/
int8_t DATA_S_List_prepend (S_List *list, S_List_Node *node);

/**
 * @brief Pops node from the beginning of a Singly Linked List
 * @returns Pointer to node from top of list on success, NULL on failure
*/
S_List_Node * DATA_S_List_pop (S_List *list);

