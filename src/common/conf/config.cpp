#include <stdint.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <regex.h>

#include "conf/config.h"
#include "util/comm.h"
#include "log/log.h"

#define LOG_CONSOLE_THRESHOLD_THIS  LOG_THRESHOLD_DEFAULT
#define LOG_FILE_THRESHOLD_THIS     LOG_THRESHOLD_MAX

Config::Config()
{
    ClearKeyVals();
}

Config::~Config()
{
    ClearKeyVals();
}

const char* Config::GetFilePath()
{
    return &path[0];
}

int Config::SetFilePath(const char* file_path)
{
    if (!file_path) STD_FAIL;
    if (strlen(file_path) >= CONF_MEMBER_LEN)
    {
        LOG_WARN("Config path too long; Max path length: %d", CONF_MEMBER_LEN);
        return -1;
    }

    strcpy(&this->path[0], file_path);
    return 0;
}

/**
 * @brief Helper function for determining line terminators
 * @returns 1 for terminator, 0 otherwise
*/
static int is_term(char ch)
{
    switch (ch)
    {
        //Intentional fallthroughs
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
 * @brief Helper function for copying the contents of a regex match to a char buffer
 * @param dst Destination char buffer
 * @param src Original string from which matches are extracted
 * @param group Match information
*/
static void copy_regex_group(char* dst, const char* src, regmatch_t* group)
{
    uint8_t match_len = group->rm_eo - group->rm_so;
    strncpy(&dst[0], &src[group->rm_so], match_len);
    dst[match_len] = '\0';
}

int Config::GetKeyIndex(const char* key)
{
    int16_t idx = -1;
    for (uint8_t iter = 0; iter < key_count; iter++)
    {
        if(0 == strcmp(key, &keys[CONF_KEY_LEN * iter]))
        {
            idx = iter;
            break;
        }
    }

    return idx;
}

int Config::ParseYaml(int fd)
{
    // Key pairs are assumed to be in "<KEY>: <VAL>" format, with a newline, null terminator, or end-of-file after each value
    // read function uses system call; minimize using buffers as much as possible

    regex_t entry;                                  // Compiled regex pattern for detecting config entries
    regmatch_t matches[3];                          // Match groups for regex
    uint8_t line = 1;                               // Tracks line number for logging/debugging
    off_t offset;                                   // Tracks how far to back track for next read; used to line up to just after the first newline in the buffer
    char buffer[CONF_KEY_LEN + CONF_VAL_LEN + 4];   // buffer to store read calls in; ": " and "\n\0" require 4 additional characters
    char key[CONF_KEY_LEN];                         // buffer to temporarily hold keys/vals
    int result;                                     // result of each read

    if (0 != regcomp(&entry, CONF_ENTRY_FMT, CONF_REGEX_FLAGS)) STD_FAIL;   // Compile regex pattern
    while(3 <= (result = read(fd, &buffer[0], sizeof(buffer) - 1)))         // Loop until file is empty
    {
        LOG_VERBOSE(5, "RESULT: %d", result);
        LOG_VERBOSE(6, "BUFFER: %s", buffer);

        buffer[result] = '\0';                                              // Null terminate buffer for str operations
        int ret = regexec(&entry, &buffer[0], 3, matches, 0);               // match regex pattern against read buffer
        if(!ret)
        {
            // Match found
            copy_regex_group(&key[0], &buffer[0], &matches[1]);
            int idx = GetKeyIndex(&key[0]);
            LOG_VERBOSE(4, "KEY: %s", key);
            LOG_VERBOSE(5, "Key Index: %d", idx);


            if (-1 == idx)
            {
                LOG_VERBOSE(4, "Unrecognized key; Moving on");
            }
            else
            {
                char* val = &vals[idx * CONF_VAL_LEN];
                copy_regex_group(val, &buffer[0], &matches[2]);
                LOG_VERBOSE(4, "VAL: %s", val);
            }

            offset = matches[0].rm_eo;  // set offset to end of match
        }
        else
        {
            LOG_VERBOSE(4, "NO MATCH ON LINE %d", line);
            for (offset = 0; offset < result && !is_term(buffer[offset]); offset++);    // Sets offset to first newline in buffer
        }

        offset -= result - 1;
        LOG_VERBOSE(6, "OFFSET: %d", offset);
        memset(buffer, 0, sizeof(buffer));      // Clear buffer
        lseek(fd, offset, SEEK_CUR);            // Align file iterator to just after first newline in buffer
        line++;                                 // Increment line counter
    }

    return 0;
}

int Config::ParseConfig()
{
    if (0 == strlen(path)) STD_FAIL;
    int fd = open(path, CONF_FILE_PERM);
    if (fd < 0)
    {
        LOG_ERROR("Could not open config: (%d) %s", errno, strerror(errno));
        return -1;
    }

    ParseYaml(fd);

    if(close(fd)) STD_FAIL;
    return 0;
}

int Config::AddKey(const char* key)
{
    if (!key) STD_FAIL;
    if (CONF_PAIR_LIMIT <= key_count) STD_FAIL;

    strcpy(&keys[CONF_KEY_LEN * key_count], key);
    return key_count++;
}

const char* Config::GetVal(uint8_t idx)
{
    if (idx >= key_count) STD_FAIL_VOID_PTR;
    return &vals[CONF_VAL_LEN * idx];
}

const char* Config::GetVal(const char* key)
{
    if (!key) STD_FAIL_VOID_PTR;

    int idx = GetKeyIndex(key);
    if (-1 == idx) return NULL;
    return GetVal(idx);
}

void Config::ClearKeyVals()
{
    memset(keys, 0, sizeof(keys));
    memset(vals, 0, sizeof(vals));
    key_count = 0;
}

int Config::OverrideValue(uint8_t key_idx, const char* val)
{
    if (key_idx > key_count) STD_FAIL;
    if (!val) STD_FAIL;

    if (strlen(val) + 1 > CONF_VAL_LEN)
        LOG_WARN("Override value for key %s too long", &keys[key_idx * CONF_KEY_LEN]);

    strcpy(&vals[key_idx * CONF_VAL_LEN], val);
    return 0;
}