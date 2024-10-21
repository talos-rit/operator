/**
 * 
*/

#pragma once
#include <stdint.h>

#include "api/api.h"

#ifdef __cplusplus
extern "C" {
#endif

// No validate_handshake; No body to validate

/**
 * @brief validates the payload of a Polar Pan command message
 * @param payload Pointer to Polar Pan command payload
 * @returns 0 on success, -1 on failure
*/
int API_prep_polar_pan(API_Data_Polar_Pan* payload);

/**
 * @brief validates the payload of a Home command message
 * @param payload Pointer to Polar Pan command payload
 * @returns 0 on success, -1 on failure
*/
int API_prep_home(API_Data_Home* payload);

#ifdef __cplusplus
}
#endif