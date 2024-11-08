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

#define LOG_CONSOLE_THRESHOLD_THIS  LOG_VERBOSE + 4
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

void Config::ParseYaml(int fd)
{
    // Key pairs are assumed to be in "<KEY>: <VAL>" format, with a newline, null terminator, or end-of-file after each value
    // read function uses system call; minimize using buffers as much as possible

    regex_t entry;
    regmatch_t matches[3];
    int ret = regcomp(&entry, "^([^:]*): (.*)$", REG_NEWLINE | REG_EXTENDED);
    LOG_VERBOSE(4, "REG COMP: %d", ret);

    char buffer[CONF_KEY_LEN + CONF_VAL_LEN + 4]; // ": " and "\n\0" require 4 additional characters
    char debug[CONF_KEY_LEN];
    int result; // result of each read
    off_t offset = 0;
    uint8_t line = 1;

    while(3 <= (result = read(fd, &buffer[0], sizeof(buffer) - 1)))
    {
        LOG_VERBOSE(4, "RESULT: %d", result);
        buffer[result] = '\0';
        int ret = regexec(&entry, &buffer[0], 3, matches, 0);
        LOG_VERBOSE(6, "BUFFER: %s", buffer);
        if(!ret)
        {
            debug[matches[1].rm_eo - matches[1].rm_so] = '\0';
            strncpy(&debug[0], &buffer[matches[1].rm_so], matches[1].rm_eo - matches[1].rm_so);
            debug[matches[1].rm_eo - matches[1].rm_so] = '\0';
            LOG_VERBOSE(4, "KEY: %s", debug);

            debug[matches[2].rm_eo - matches[2].rm_so] = '\0';
            strncpy(&debug[0], &buffer[matches[2].rm_so], matches[2].rm_eo - matches[2].rm_so);
            debug[matches[1].rm_eo - matches[2].rm_so] = '\0';
            LOG_VERBOSE(4, "VAL: %s", debug);
            // Match found; Check key
            // If key is present, fill value

            offset = -(result - matches[2].rm_eo) + 1;
        }
        else
        {
            LOG_VERBOSE(4, "NO MATCH ON LINE %d", line);
            uint8_t term_idx = 0;
            for (; term_idx < result && !is_term(buffer[term_idx]); term_idx++);
            offset = -(result - term_idx);
        }
        LOG_VERBOSE(6, "OFFSET: %d", offset);
        memset(buffer, 0, sizeof(buffer));
        lseek(fd, offset, SEEK_CUR);
        line++;
    }
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

int Config::GetVal(char* dst, uint8_t idx)
{
    if (!dst) STD_FAIL;
    if (idx >= key_count) STD_FAIL;

    strcpy(dst, &keys[CONF_KEY_LEN * idx]);
    return 0;
}

int Config::GetVal(char* dst, const char* key)
{
    if (!dst) STD_FAIL;
    if (!key) STD_FAIL;

    // Linear search for key
    int16_t idx = -1;
    for (uint8_t iter = 0; iter < key_count; iter++)
    {
        if(0 == strcmp(key, &keys[CONF_KEY_LEN * iter]))
        {
            idx = iter;
            break;
        }
    }

    if (-1 == idx) return -1;
    return GetVal(dst, idx);
}

void Config::ClearKeyVals()
{
    memset(keys, 0, sizeof(keys));
    memset(vals, 0, sizeof(vals));
    key_count = 0;
}
