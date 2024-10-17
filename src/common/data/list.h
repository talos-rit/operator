#pragma once

#define DATA_LIST_GET_OBJ(node, obj_type, node_name) ((obj_type*)(((uint8_t*) node) - offsetof(obj_type, node_name)))