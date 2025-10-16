#include "tamq/tamq_conf.h"

#include <stdint.h>
#include <string.h>

#include "util/comm.h"

#define LOG_CONSOLE_THRESHOLD_THIS LOG_THRESHOLD_DEFAULT
#define LOG_FILE_THRESHOLD_THIS LOG_THRESHOLD_DEFAULT

TAMQ_Config::TAMQ_Config() {
  address_idx = AddKey(TAMQ_CONF_ADDR_KEY, TAMQ_CONF_ADDR_DEFAULT);
  cmd_uri_idx = AddKey(TAMQ_CONF_CMD_URI_KEY, TAMQ_CONF_CMD_URI_DEFAULT);
  ret_uri_idx = AddKey(TAMQ_CONF_RET_URI_KEY, TAMQ_CONF_RET_URI_DEFAULT);
  log_uri_idx = AddKey(TAMQ_CONF_LOG_URI_KEY, TAMQ_CONF_LOG_URI_DEFAULT);

  use_topics_idx =
      AddKey(TAMQ_CONF_USE_TOPICS_KEY, TAMQ_CONF_USE_TOPICS_DEFAULT);
  client_ack_idx =
      AddKey(TAMQ_CONF_CLIENT_ACK_KEY, TAMQ_CONF_CLIENT_ACK_DEFAULT);
}

TAMQ_Config::~TAMQ_Config() {}

const char *TAMQ_Config::GetBrokerAddress() { return GetVal(address_idx); }

const char *TAMQ_Config::GetCommandURI() { return GetVal(cmd_uri_idx); }

const char *TAMQ_Config::GetReturnsURI() { return GetVal(ret_uri_idx); }

const char *TAMQ_Config::GetLogURI() { return GetVal(log_uri_idx); }

bool TAMQ_Config::GetUseTopics() { return GetBool(use_topics_idx); }

bool TAMQ_Config::GetClientAck() { return GetBool(client_ack_idx); }

int TAMQ_Config::LoadDefaults() {
  int8_t counter = 0;
  counter -= OverrideValue(address_idx, TAMQ_CONF_ADDR_DEFAULT);
  counter -= OverrideValue(cmd_uri_idx, TAMQ_CONF_CMD_URI_DEFAULT);
  counter -= OverrideValue(ret_uri_idx, TAMQ_CONF_RET_URI_DEFAULT);
  counter -= OverrideValue(log_uri_idx, TAMQ_CONF_LOG_URI_DEFAULT);

  counter -= OverrideValue(use_topics_idx, TAMQ_CONF_USE_TOPICS_DEFAULT);
  counter -= OverrideValue(client_ack_idx, TAMQ_CONF_CLIENT_ACK_DEFAULT);

  return counter ? -1 : 0;
}