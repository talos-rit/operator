#include <endian.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "api/api.h"
#include "log/log.h"
#include "sub/sub.h"
#include "util/array.h"
#include "util/comm.h"

#define LOG_FILE_THRESHOLD_THIS LOG_THRESHOLD_MAX
#define LOG_CONSOLE_THRESHOLD_THIS LOG_THRESHOLD_MAX

/**
 * @brief Helper function for simulating incoming message header
 * @param header Command header to prepare
 */
static void prep_header(API_Data_Header *header) {
  header->cmd_id = htobe32(header->cmd_id);
  header->cmd_val = htobe16(header->cmd_val);
  header->len = htobe16(header->len);
}

/**
 * @brief Helper function for simulating incoming polar pan message
 * @param payload Payload to prep
 */
static void prep_polar_pan(API_Data_Polar_Pan *payload) {
  payload->delta_altitude = htobe32(payload->delta_altitude);
  payload->delta_azimuth = htobe32(payload->delta_azimuth);
  payload->delay_ms = htobe32(payload->delay_ms);
  payload->time_ms = htobe32(payload->time_ms);
}

/**
 * @brief Helper function for simulating incoming command
 * @param cmd Command Wrapper
 */
static void prep_polar_pan_wrapper(API_Data_Wrapper *cmd) {
  prep_header(&cmd->header);
  prep_polar_pan((API_Data_Polar_Pan *)&cmd->payload_head);
}

/**
 * @brief fills byte array with simulated data
 * @param bytes Byte array
 */
static void fill_simulated_command(uint8_t *bytes) {
  API_Data_Wrapper *cmd = (API_Data_Wrapper *)bytes;
  API_Data_Header *header = (API_Data_Header *)&cmd->header;
  API_Data_Polar_Pan *payload = (API_Data_Polar_Pan *)&cmd->payload_head;

  // Assign test values to header
  header->cmd_id = 0xBE;
  header->cmd_val = API_CMD_POLARPAN;
  header->len = sizeof(API_Data_Polar_Pan);

  // Assign test values to body
  payload->delta_azimuth = -1;
  payload->delta_altitude = 0;
  payload->delay_ms = 1;
  payload->time_ms = 2;
}

int main() {
  LOG_init();
  LOG_start();
  LOG_INFO("Staring demo...");

  // Setup buffers
  uint16_t byte_len = sizeof(API_Data_Header) + sizeof(API_Data_Polar_Pan) + 2;
  uint8_t bytes[byte_len];
  memset(&bytes[0], 0, byte_len);
  char text[UTIL_BYTE_STR_FMT_LEN(byte_len)];
  memset(&text[0], 0, UTIL_BYTE_STR_FMT_LEN(byte_len));

  // Prep simulated incoming message
  fill_simulated_command(&bytes[0]);
  API_Data_Wrapper *cmd = (API_Data_Wrapper *)&bytes[0];
  UTIL_format_byte_str(&text[0], &bytes[0], byte_len);
  LOG_VERBOSE(4, "Init: %s", text);

  prep_polar_pan_wrapper(cmd);

  UTIL_format_byte_str(&text[0], &bytes[0], byte_len);
  LOG_VERBOSE(4, "Pre : %s", text);

  // process command using API module
  if (API_validate_command(&bytes[0], byte_len)) LOG_IEC();

  UTIL_format_byte_str(&text[0], &bytes[0], byte_len);
  LOG_VERBOSE(4, "Post: %s", text);

  LOG_INFO("Demo Ending...");
  LOG_stop();
  LOG_destory();
}