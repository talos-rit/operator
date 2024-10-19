#include "data/s_list.h"

#include <string.h>

#include "log/log.h"

#define LOG_CONSOLE_THRESHOLD_THIS LOG_VERBOSE + 2
#define LOG_FILE_THRESHOLD_THIS LOG_VERBOSE + 2

int8_t DATA_S_List_Node_init (S_List_Node *node)
{
    if (!node) STD_FAIL;
    memset(node, 0, sizeof(S_List_Node));
    return 0;
}

int8_t DATA_S_List_init (S_List *list)
{
    if (!list) STD_FAIL;
    memset(list, 0, sizeof(S_List));
    return 0;
}

int8_t DATA_S_List_deinit (S_List *list)
{
    if (!list) STD_FAIL;
    while (list->len > 0)
    {
        // Pop head; preserve list in case of error
        S_List_Node *iter = DATA_S_List_pop(list);
        if (!iter && list->len > 0) STD_FAIL;

        // Reinit purged node
        DATA_S_List_Node_init (iter);
    }
    
    if (list->head) STD_FAIL
    if (list->tail) STD_FAIL
    if (list->len)  STD_FAIL;
    return 0;
}

int8_t DATA_S_List_append (S_List *list, S_List_Node *node)
{
    if (!list) STD_FAIL;
    if (node->next) STD_FAIL;

    if (list->len == 0) list->head = node;
    else list->tail->next = node;

    list->tail = node;
    list->len++;
    return 0;
}

int8_t DATA_S_List_insert (S_List *list, S_List_Node *node, uint16_t index)
{
    if (!list || !node) STD_FAIL;
    if (index > list->len) STD_FAIL;
    if (index == list->len)
    {
        int8_t ret = DATA_S_List_append (list, node);
        if (ret == -1) STD_FAIL
        else return 0;
    }

    // Guranteed to not be last
    S_List_Node **link = &list->head;
    S_List_Node *n_iter = list->head;
    for(uint16_t c_iter = 0; c_iter < index && n_iter; c_iter++)
    {
        link = &n_iter->next;
        n_iter = n_iter->next;
    }

    if (!link) STD_FAIL;
    node->next = n_iter;
    *link = node;
    return 0;
}

int8_t DATA_S_List_prepend (S_List *list, S_List_Node *node)
{
    if (!list) STD_FAIL;
    if (node->next) STD_FAIL;
    if (DATA_S_List_insert(list, node, 0) == -1) STD_FAIL;
    return 0;
}

S_List_Node *DATA_S_List_pop (S_List *list)
{
    if (!list || list->len == 0) STD_FAIL_VOID_PTR

    S_List_Node *node = list->head;
    list->head = list->head->next;
    list->len--;
    if (list->len == 0) list->tail = list->head = NULL;
    DATA_S_List_Node_init(node);
    return node;
}
