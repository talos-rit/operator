#pragma once

#include "conf/config.h"

// Set the URI to point to the IPAddress of your broker.
// add any optional params to the url to enable things like
// tightMarshalling or tcp logging etc.  See the CMS web site for
// a full list of configuration options.
//
//  http://activemq.apache.org/cms/
//
#define TAMQ_CONF_ADDR_DEFAULT "failover:(tcp://127.0.0.1:61616)"

//============================================================
// This is the Destination Name and URI options.  Use this to
// customize where the consumer listens, to have the consumer
// use a topic or queue set the 'useTopics' flag.
//============================================================
#define TAMQ_CONF_CMD_URI_DEFAULT "instructions"
#define TAMQ_CONF_RET_URI_DEFAULT "returns"
#define TAMQ_CONF_LOG_URI_DEFAULT "logs"

//============================================================
// set to true to use topics instead of queues
// Note in the code above that this causes createTopic or
// createQueue to be used in the consumer.
//============================================================
#define TAMQ_CONF_USE_TOPICS_DEFAULT false

//============================================================
// set to true if you want the consumer to use client ack mode
// instead of the default auto ack mode.
//============================================================
#define TAMQ_CONF_CLIENT_ACK_DEFAULT false

#define TAMQ_CONF_ADDR_KEY "amq_address"
#define TAMQ_CONF_CMD_URI_KEY "amq_cmd_uri"
#define TAMQ_CONF_RET_URI_KEY "amq_ret_uri"
#define TAMQ_CONF_LOG_URI_KEY "amq_log_uri"
#define TAMQ_CONF_USE_TOPICS_KEY "amq_use_topics"
#define TAMQ_CONF_CLIENT_ACK_KEY "amq_client_ack"

class TAMQ_Config : virtual public Config {
 private:
  int address_idx;    /** IP of the ActiveMQ broker */
  int cmd_uri_idx;    /** Topic/Queue for incoming commands */
  int ret_uri_idx;    /** Topic/Queue for return messages */
  int log_uri_idx;    /** Topic/Queue for logs */
  int use_topics_idx; /** Determines if topics will be used, rather than queues
                       */
  int client_ack_idx; /** Determines if client will send ACK message, or if it
                         will automatically be sent */

 public:
  TAMQ_Config();
  ~TAMQ_Config();

  /**
   * @brief Gets config for ActiveMQ broker address
   * @returns config string on success, NULL on failure
   */
  const char *GetBrokerAddress();

  /**
   * @brief Sets config for ActiveMQ broker address
   * @param addr Broker address to set in config
   * @returns 0 on success, -1 on failure
   */
  int SetBrokerAddress(const char *addr);

  /**
   * @brief Gets config for Command URI
   * @returns config string on success, NULL on failure
   */
  const char *GetCommandURI();

  /**
   * @brief Sets config for Command URI
   * @param cmd_uri Command URI to set in config
   * @returns 0 on success, -1 on failure
   */
  int SetCommandURI(const char *cmd_uri);

  /**
   * @brief Gets config for Returns URI
   * @returns config string on success, NULL on failure
   */
  const char *GetReturnsURI();

  /**
   * @brief Sets config for Returns URI
   * @param ret_uri Returns URI to set in config
   * @returns 0 on success, -1 on failure
   */
  int SetReturnsURI(const char *ret_uri);

  /**
   * @brief Gets config for Log URI
   * @returns config string on success, NULL on failure
   */
  const char *GetLogURI();

  /**
   * @brief Sets config for Log URI
   * @param log_uri Log URI to set in config
   * @returns 0 on success, -1 on failure
   */
  int SetLogURI(const char *log_uri);

  /**
   * @brief Gets config for Use Topics setting
   * @returns config string on success, NULL on failure
   */
  bool GetUseTopics();

  /**
   * @brief Sets config for Use Topics setting
   * @param use_topics Set to true to use topics; Set to false to use queues
   * @returns 0 on success, -1 on failure
   */
  int SetUseTopics(bool use_topics);

  /**
   * @brief Gets config for Client ACK setting
   * @returns config string on success, NULL on failure
   */
  bool GetClientAck();

  /**
   * @brief Sets config for Client ACK setting
   * @param client_ack Set to true to have client manually ACK broker; Set to
   * false to auto ACK broker
   * @returns 0 on success, -1 on failure
   */
  int SetClientAck(bool client_ack);

  int LoadDefaults();
};