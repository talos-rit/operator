#pragma once

/**
 * @brief Returns a pointer for the parent structure of a node
 * @details requires #include <stddef.h>
 * @param node Pointer to node
 * @param obj_type Type of desired structure
 * @param node_name Name of the node member within the parent structure
 * @returns Pointer to parent object (type obj_type*) on success, NULL on failure
*/
#define DATA_LIST_GET_OBJ(node, obj_type, node_name) ((obj_type*)(((uint8_t*) node) - offsetof(obj_type, node_name)))