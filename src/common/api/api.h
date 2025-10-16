/**
 * ICD Compliant Message processor
 */

#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum _api_cmd_id {
  API_CMD_HANDSHAKE = 0x0000,
  API_CMD_POLARPAN = 0x0001,
  API_CMD_HOME = 0x0002,
  API_CMD_POLARPAN_START = 0x0003,
  API_CMD_POLARPAN_STOP = 0x0004,
  API_CMD_SET_SPEED = 0x0005,
  API_CMD_GET_SPEED = 0x0006,
} API_Command_ID;

#pragma pack(push, 1)
/** Struct mapping command message header values */
typedef struct _api_data_wrapper_header {
  uint32_t cmd_id;     /** Unique ID for individual commands */
  uint16_t reserved_1; /** RESERVED */
  uint16_t cmd_val;    /** Command for device to carry out */
  uint16_t len;        /** Length of Payload */
} API_Data_Header;

/** Wrapper Struct */
typedef struct _api_data_wrapper {
  API_Data_Header header;
  uint8_t payload_head; /** Command Info */
} API_Data_Wrapper;

typedef struct _api_data_polar_pan {
  int32_t delta_azimuth;  /** Requested change in azimuth */
  int32_t delta_altitude; /** Requested change in altitude */
  uint32_t delay_ms;      /** How long to wait until executing pan */
  uint32_t time_ms;       /** How long the pan should take to execute */
} API_Data_Polar_Pan;

typedef struct _api_data_polar_pan_start {
  int8_t delta_azimuth;  /** Requested change in azimuth */
  int8_t delta_altitude; /** Requested change in altitude */
} API_Data_Polar_Pan_Start;

typedef struct _api_data_home {
  uint32_t delay_ms; /** How long to wait until executing pan */
} API_Data_Home;

typedef struct _api_data_set_speed {
  uint8_t speed; /** Speed value 0-100 */
} API_Data_Set_Speed;

#pragma pack(pop)

#define API_WRAPPER_HEAD_LEN offsetof(API_Data_Wrapper, payload_head)

/**
 * @brief Checks to see if a command is valid and can be processed safely
 * @details Fixes endianness, Checks validity of inputs,
 * and the length and CRC of the provided input
 * @param buf Byte buffer
 * @param len Length of byte buffer
 * @returns 0 on success, -1 on failure
 */
int API_validate_command(const uint8_t *buf, uint16_t len);

#ifdef __cplusplus
}
#endif
