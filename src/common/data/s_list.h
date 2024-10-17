#pragma once

#include <stdint.h>

#include "data/list.h"

typedef struct _s_list_node
{
    struct _s_list_node *next;
} S_List_Node;

typedef struct _s_list
{
    S_List_Node *head;
    S_List_Node *tail;  // Not standard/necessary, but this is commonly used as a queue
    uint32_t len;
} S_List;

int8_t DATA_S_List_Node_init (S_List_Node *node);

int8_t DATA_S_List_init (S_List *list);

int8_t DATA_S_List_deinit (S_List *list);

int8_t DATA_S_List_append (S_List *list, S_List_Node *node);

int8_t DATA_S_List_insert (S_List *list, S_List_Node *node, uint16_t index);

int8_t DATA_S_List_prepend (S_List *list, S_List_Node *node);

S_List_Node * DATA_S_List_pop (S_List *list);

