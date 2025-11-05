/**
 * ICD Compliant Message processor
 */

#pragma once

#include <cstddef>
#include <cstdint>

namespace API {

enum class CommandID {
  Handshake = 0x0000,
  PolarPan = 0x0001,
  Home = 0x0002,
  PolarPanStart = 0x0003,
  PolarPanStop = 0x0004,
};

#pragma pack(push, 1)
/** Struct mapping command message header values */
struct DataHeader {
  uint32_t msg_id;     /** Unique ID for individual commands */
  uint16_t reserved_1; /** RESERVED */
  uint16_t cmd_id;    /** Command for device to carry out */
  uint16_t len;        /** Length of Payload */
};

/** Wrapper Struct */
struct DataWrapper {
  DataHeader header;
  std::byte payload_head; /** Command Info */
};

namespace Requests {
  struct PolarPan {
    int32_t delta_azimuth;  /** Requested change in azimuth */
    int32_t delta_altitude; /** Requested change in altitude */
    uint32_t delay_ms;      /** How long to wait until executing pan */
    uint32_t time_ms;       /** How long the pan should take to execute */
  };

  struct PolarPanStart {
    int8_t delta_azimuth;  /** Requested change in azimuth */
    int8_t delta_altitude; /** Requested change in altitude */
  };

  struct Home {
    uint32_t delay_ms; /** How long to wait until executing pan */
  };
}

namespace Responses {
  struct Simple {
    uint16_t return_code; /** Status code of executed command */
  };
}

#pragma pack(pop)

constexpr size_t WRAPPER_HEAD_LEN = offsetof(DataWrapper, payload_head);

/**
 * @brief Checks to see if a command is valid and can be processed safely
 * @details Fixes endianness, Checks validity of inputs,
 * and the length and CRC of the provided input
 * @param buf Byte buffer
 * @param len Length of byte buffer
 * @returns 0 on success, -1 on failure
 */
int validate_command(const uint8_t *buf, uint16_t len);

}  // namespace API
