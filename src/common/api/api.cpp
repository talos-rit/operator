#include "api/api.hpp"

#include <endian.h>
#include <stddef.h>
#include <stdlib.h>

#include "util/comm.hpp"

#define LOG_CONSOLE_THRESHOLD_THIS LOG_THRESHOLD_MAX
#define LOG_FILE_THRESHOLD_THIS LOG_THRESHOLD_MAX

namespace API {

int prep_polar_pan(API::PolarPan* payload) {
  if (!payload) STD_FAIL;

  payload->delta_altitude = be32toh(payload->delta_altitude);
  payload->delta_azimuth = be32toh(payload->delta_azimuth);
  payload->delay_ms = be32toh(payload->delay_ms);
  payload->time_ms = be32toh(payload->time_ms);

  return 0;
}

int prep_home(API::Home* payload) {
  if (!payload) STD_FAIL;

  payload->delay_ms = be32toh(payload->delay_ms);

  return 0;
}

int validate_command(const uint8_t* buf, uint16_t len) {
  // Check inputs
  if (!buf) STD_FAIL;
  auto* cmd = reinterpret_cast<DataWrapper*>(const_cast<uint8_t*>(buf));

  // Fix endianness
  cmd->header.msg_id = be32toh(cmd->header.msg_id);
  cmd->header.cmd_id = be16toh(cmd->header.cmd_id);
  cmd->header.len = be16toh(cmd->header.len);

  // TODO: Check for duplicate cmd_ids

  // Check length
  if (len < sizeof(API::DataHeader) + cmd->header.len) return -1;

  // Command specific preparation
  switch (static_cast<API::CommandID>(cmd->header.cmd_id)) {
    // Intentional fallthrough
    case API::CommandID::Handshake:
      // No body; Always valid
    case API::CommandID::PolarPanStart:
      // Already correct endianness
    case API::CommandID::PolarPanStop:
      // Already correct endianness
      break;
    case API::CommandID::PolarPan:
      prep_polar_pan((API::PolarPan*)&cmd->payload_head);
      break;
    case API::CommandID::Home:
      prep_home((API::Home*)&cmd->payload_head);
      break;
    case API::CommandID::ExecuteHardwareOperation: {
      if (cmd->header.len < sizeof(API::HardwareOperation)) return -1;
      auto* operation = (API::HardwareOperation*)&cmd->payload_head;
      switch (static_cast<API::HardwareOperationID>(operation->subcommand)) {
        case API::HardwareOperationID::JointJogStart: {
          if (cmd->header.len !=
              sizeof(API::HardwareOperation) + sizeof(API::JointJogStart))
            return -1;
          auto* jog = reinterpret_cast<API::JointJogStart*>(operation + 1);
          if ((jog->axis != 2 && jog->axis != 3) ||
              (jog->direction != -1 && jog->direction != 1))
            return -1;
          break;
        }
        case API::HardwareOperationID::JointJogStop:
          if (cmd->header.len != sizeof(API::HardwareOperation)) return -1;
          break;
        case API::HardwareOperationID::JointMoveRelative:
          if (cmd->header.len != sizeof(API::HardwareOperation) + sizeof(API::JointMoveRelative)) return -1;
          break;
        default:
          return -1;
      }
      break;
    }
    default:
      LOG_ERROR(
          "API: Failed to process Command ID %d: Unrecognized Command Value: "
          "%d",
          cmd->header.msg_id, cmd->header.cmd_id);
      return -1;
  }

  // Check CRC
  // TODO: Implement
  uint8_t* crc = (uint8_t*)(&cmd->payload_head + cmd->header.len);

  return 0;
}
}  // namespace API
