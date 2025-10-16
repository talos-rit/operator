#include "api/api.h"

#include <endian.h>
#include <stddef.h>
#include <stdlib.h>

#include "api/api_private.h"
#include "util/array.h"
#include "util/comm.h"

#define LOG_CONSOLE_THRESHOLD_THIS LOG_THRESHOLD_MAX
#define LOG_FILE_THRESHOLD_THIS LOG_THRESHOLD_MAX

int API_prep_polar_pan(API_Data_Polar_Pan *payload) {
  if (!payload) STD_FAIL;

  payload->delta_altitude = be32toh(payload->delta_altitude);
  payload->delta_azimuth = be32toh(payload->delta_azimuth);
  payload->delay_ms = be32toh(payload->delay_ms);
  payload->time_ms = be32toh(payload->time_ms);

  return 0;
}

int API_prep_home(API_Data_Home *payload) {
  if (!payload) STD_FAIL;

  payload->delay_ms = be32toh(payload->delay_ms);

  return 0;
}

int API_validate_command(const uint8_t *buf, uint16_t len) {
  // Check inputs
  if (!buf) STD_FAIL;
  API_Data_Wrapper *cmd = (API_Data_Wrapper *)buf;

  // Fix endianness
  cmd->header.cmd_id = be32toh(cmd->header.cmd_id);
  cmd->header.cmd_val = be16toh(cmd->header.cmd_val);
  cmd->header.len = be16toh(cmd->header.len);

  // TODO: Check for duplicate cmd_ids

  // Check length
  if (len < sizeof(API_Data_Header) + cmd->header.len) STD_FAIL

  // Command specific preparation
  switch (cmd->header.cmd_val) {
    // Intentional fallthrough
    case API_CMD_HANDSHAKE:
      // No body; Always valid
    case API_CMD_POLARPAN_START:
      // Already correct endianness
    case API_CMD_POLARPAN_STOP:
      // Already correct endianness
      break;

    case API_CMD_POLARPAN:
      API_prep_polar_pan((API_Data_Polar_Pan *)&cmd->payload_head);
      break;
    case API_CMD_HOME:
      API_prep_home((API_Data_Home *)&cmd->payload_head);
      break;
    default:
      LOG_ERROR(
          "API: Failed to process Command ID %d: Unrecognized Command Value: "
          "%d",
          cmd->header.cmd_id, cmd->header.cmd_val);
      return -1;
  }

  // Check CRC
  // TODO: Implement
  uint16_t *crc = (uint16_t *)(&cmd->payload_head + cmd->header.len);
  *crc = be16toh(*crc);

  return 0;
}