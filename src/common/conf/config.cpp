#include "conf/config.h"

#include <errno.h>
#include <fcntl.h>
#include <regex.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "log/log.h"
#include "util/comm.h"

#define LOG_CONSOLE_THRESHOLD_THIS LOG_THRESHOLD_DEFAULT
#define LOG_FILE_THRESHOLD_THIS LOG_THRESHOLD_MAX

Config::Config() {
  conf_errno = 0;
  ClearKeyVals();
}

Config::~Config() { ClearKeyVals(); }

/**
 * @brief Helper function to initalize configuration entry
 * @param entry Entry to initialize
 * @returns 0 on success, -1 on failure
 */
static int CONF_Entry_init(CONF_Entry *entry) {
  if (!entry) STD_FAIL;
  memset(entry, 0, sizeof(CONF_Entry));
  return 0;
}

const char *Config::GetFilePath() { return &path[0]; }

int Config::SetFilePath(const char *file_path) {
  if (!file_path) STD_FAIL;

  if (strlen(file_path) >= CONF_MEMBER_LEN) {
    LOG_WARN("Config path too long; Max path length: %d", CONF_MEMBER_LEN);
    return -1;
  }

  if (access(file_path, F_OK)) {
    LOG_WARN("Error setting config file: \"%s\": Does not exist.", file_path);
    return -1;
  }

  if (access(file_path, R_OK)) {
    LOG_WARN("Error setting config file: \"%s\": Permission denied.",
             file_path);
    return -1;
  }

  strcpy(&this->path[0], file_path);
  LOG_VERBOSE(0, "Config file set: %s", file_path);
  return 0;
}

/**
 * @brief Helper function for determining line terminators
 * @returns 1 for terminator, 0 otherwise
 */
static int is_term(char ch) {
  switch ((signed char)ch) {
    // Intentional fallthroughs
    case '\n':
    case '\0':
    case EOF:
      return 1;
    default:
      break;
  }
  return 0;
}

/**
 * @brief Helper function for copying the contents of a regex match to a char
 * buffer
 * @param dst Destination char buffer
 * @param src Original string from which matches are extracted
 * @param group Match information
 */
static void copy_regex_group(char *dst, const char *src, regmatch_t *group) {
  uint8_t match_len = group->rm_eo - group->rm_so;
  strncpy(&dst[0], &src[group->rm_so], match_len);
  dst[match_len] = '\0';
}

int Config::GetKeyIndex(const char *key) {
  int16_t idx = -1;
  for (uint8_t iter = 0; iter < key_count; iter++) {
    if (0 == strcmp(key, &pairs[iter].key[0])) {
      idx = iter;
      break;
    }
  }

  return idx;
}

int Config::ParseYaml(int fd) {
  // Key pairs are assumed to be in "<KEY>: <VAL>" format, with a newline, null
  // terminator, or end-of-file after each value read function uses system call;
  // minimize using buffers as much as possible

  regex_t entry;          // Compiled regex pattern for detecting config entries
  regmatch_t matches[3];  // Match groups for regex
  uint8_t line = 1;       // Tracks line number for logging/debugging
  off_t offset;  // Tracks how far to back track for next read; used to line up
                 // to just after the first newline in the buffer
  char buffer[CONF_KEY_LEN + CONF_VAL_LEN +
              4];  // buffer to store read calls in; ": " and "\n\0" require 4
                   // additional characters
  char key[CONF_KEY_LEN];  // buffer to temporarily hold keys/vals
  int result = 0;          // result of each read

  memset(&buffer[0], 0, sizeof(buffer));
  if (0 != regcomp(&entry, CONF_ENTRY_FMT, CONF_REGEX_FLAGS))
    STD_FAIL;  // Compile regex pattern
  while (3 <= (result = read(fd, &buffer[0],
                             sizeof(buffer) - 1)))  // Loop until file is empty
  {
    LOG_VERBOSE(5, "RESULT: %d", result);
    LOG_VERBOSE(6, "BUFFER: %s", buffer);

    buffer[result] = '\0';  // Null terminate buffer for str operations
    int ret = regexec(&entry, &buffer[0], 3, matches,
                      0);  // match regex pattern against read buffer
    if (!ret) {
      // Match found
      copy_regex_group(&key[0], &buffer[0], &matches[1]);
      int idx = GetKeyIndex(&key[0]);
      LOG_VERBOSE(4, "KEY: %s", key);
      LOG_VERBOSE(5, "Key Index: %d", idx);

      if (-1 == idx) {
        LOG_VERBOSE(4, "Unrecognized key; Moving on");
      } else {
        char *val = &pairs[idx].val[0];
        copy_regex_group(val, &buffer[0], &matches[2]);
        LOG_VERBOSE(4, "VAL: %s", val);
      }

      offset = matches[0].rm_eo;  // set offset to end of match
    } else {
      LOG_VERBOSE(4, "NO MATCH ON LINE %d", line);
      for (offset = 0; offset < result && !is_term(buffer[offset]);
           offset++);  // Sets offset to first newline in buffer
    }

    offset -= result - 1;
    LOG_VERBOSE(6, "OFFSET: %d", offset);
    memset(buffer, 0, sizeof(buffer));  // Clear buffer
    lseek(
        fd, offset,
        SEEK_CUR);  // Align file iterator to just after first newline in buffer
    line++;         // Increment line counter
  }

  regfree(&entry);
  return 0;
}

int Config::ParseConfig() {
  if (0 == strlen(path)) STD_FAIL;
  int fd = open(path, CONF_FILE_PERM);
  if (fd < 0) {
    conf_errno = errno;
    return -1;
  }

  ParseYaml(fd);

  if (close(fd)) STD_FAIL;
  return 0;
}

int Config::AddKey(const char *key, const char *deflt, CONF_Data_Type type) {
  if (!key) STD_FAIL;
  if (!deflt) STD_FAIL;
  if (CONF_PAIR_LIMIT <= key_count) STD_FAIL;

  pairs[key_count].type = type;
  strcpy(&pairs[key_count].key[0], key);
  strcpy(&pairs[key_count].val[0], deflt);
  return key_count++;
}

int Config::AddKey(const char *key, const char *deflt) {
  return AddKey(key, deflt, CONF_DATA_STRING);
}

int Config::AddKey(const char *key, int deflt) {
  char val[10];
  sprintf(&val[0], "%d", deflt);
  return AddKey(key, val, CONF_DATA_INT);
}

int Config::AddKey(const char *key, bool deflt) {
  return AddKey(key, deflt ? "true" : "false", CONF_DATA_BOOL);
}

const char *Config::GetVal(uint8_t idx) {
  if (idx >= key_count) STD_FAIL_VOID_PTR;
  return &pairs[idx].val[0];
}

const char *Config::GetVal(const char *key) {
  if (!key) STD_FAIL_VOID_PTR;

  int idx = GetKeyIndex(key);
  if (-1 == idx) return NULL;
  return GetVal(idx);
}

bool Config::fail_get_bool(int idx) {
  // Ensure in bounds
  if (idx >= 0 && idx < key_count) {
    // Must be invalid value; not "true" or "false"
    if (OverrideValue(idx, CONF_DEFAULT_BOOL_VAL)) LOG_IEC();
    LOG_WARN(
        "Unrecognized configuration value for key: \"%s\"; Using default "
        "value: %s%",
        pairs[idx].key, pairs[idx].val);
  }

  return CONF_DEFAULT_BOOL_VAL;
}

bool Config::GetBool(int idx) {
  if (idx < 0 || idx >= key_count) {
    LOG_IEC();
    return fail_get_bool(idx);
  }

  const char *val = GetVal(idx);
  bool set;

  if (0 == strcmp(val, "true"))
    set = true;

  else if (0 == strcmp(val, "false"))
    set = false;

  else
    set = fail_get_bool(idx);

  return set;
}

bool Config::GetBool(const char *key) {
  if (!key) STD_FAIL;
  int idx = GetKeyIndex(key);
  if (-1 == idx) STD_FAIL;
  return GetBool(idx);
}

int Config::GetInt(int idx) {
  const char *val = GetVal(idx);
  if (!val) STD_FAIL;
  return atoi(val);
}

int Config::GetInt(const char *key) {
  if (!key) STD_FAIL;
  int idx = GetKeyIndex(key);
  if (-1 == idx) STD_FAIL;
  return GetInt(idx);
}

void Config::ClearKeyVals() {
  for (uint8_t iter = 0; iter < CONF_PAIR_LIMIT; iter++)
    CONF_Entry_init(&pairs[iter]);

  key_count = 0;
}

int Config::OverrideValue(uint8_t key_idx, const char *val) {
  if (key_idx > key_count) STD_FAIL;
  if (!val) STD_FAIL;

  if (strlen(val) + 1 > CONF_VAL_LEN) {
    LOG_ERROR("Override value for key %s too long", &pairs[key_idx].key[0]);
    return -1;
  }

  strcpy(&pairs[key_idx].val[0], val);
  return 0;
}

int Config::OverrideValue(uint8_t key_idx, bool val) {
  const char *str_val = val ? "true" : "false";
  return OverrideValue(key_idx, str_val);
}

void Config::DumpToLog(int log_level) {
  const char *delim =
      "=============================================================";
  LOG_write(log_level, delim);
  LOG_write(log_level, "Config parameters:");

  if (conf_errno || strlen(path) == 0) {
    if (conf_errno)
      LOG_ERROR("Error opening config: (%d) %s", conf_errno,
                strerror(conf_errno));
    else if (strlen(path) == 0)
      LOG_ERROR("Error opening config: failed to set config path");
    LOG_WARN("Using default values");
  }
  LOG_write(log_level, "path: %s", path);
  for (uint8_t iter = 0; iter < key_count; iter++) {
    LOG_write(log_level, "%s: %s", pairs[iter].key, pairs[iter].val);
  }
  LOG_write(log_level, delim);
}