/**
 * Logging Module
 * Creates realtime logs to help the debugging process
 * 
 * Features:
 *      - Console and File logging
 *      - Prioritization levels
 *      - Adjustable priority thresholds for console and file log outputs on a file-by-file basis
 *      - ISO6801 Timestamps
 *      - Optional color-coding
 *      - Non-blocking (runs in separate thread)
*/

#pragma once

#include <pthread.h>
#include <stdint.h>
// #include <stdatomic.h>

#include "log/log_ansi_color.h"
#include "data/s_list.h"

#define LOG_USE_LOCATION false
#define LOG_USE_COLOR true
#define LOG_USE_STDERR true
#define LOG_FILE_PATH "/home/brooke/Dev/personal/Chariot/build/chariot.log"

#define STD_FAIL            {LOG_IEC(); return -1;}     // Not quite sure where to put this
#define STD_FAIL_VOID       {LOG_IEC(); return;}        // Not quite sure where to put this
#define STD_FAIL_VOID_PTR   {LOG_IEC(); return NULL;}   // Not quite sure where to put this

// Location info is considered part of the message body
#define LOG_MAX_BUFFER 127
#define LOG_BUFFER_LEN 255
#define LOG_HEADER_LEN 27 
#define LOG_MESSAGE_LEN (LOG_BUFFER_LEN - LOG_HEADER_LEN)

// Preprocessor abuse
#define LOG_LEVEL_ENTRIES       \
    LOG_LEVEL_ENTRY(FATAL),     \
    LOG_LEVEL_ENTRY(ERROR),     \
    LOG_LEVEL_ENTRY(WARN),      \
    LOG_LEVEL_ENTRY(INFO),      \
    LOG_LEVEL_ENTRY(VERBOSE)    // Always last

#define LOG_LEVEL_ENTRY(entry) LOG_ ## entry
typedef enum _log_level
{
    LOG_LEVEL_ENTRIES
} LOG_level;


extern const char* const _LOG_level_color[];
extern const char* const log_level_names[];

// Threshold shortcuts

#define LOG_THRESHOLD_DEFAULT               (LOG_INFO)
#define LOG_THRESHOLD_MAX                   (LOG_VERBOSE + 9)
#define LOG_THRESHOLD_QUIET                 (LOG_ERROR)
#define LOG_THRESHOLD_SILENT                (LOG_FATAL - 1)

// Log shortcuts

#define LOG_FATAL(fmt, ...)                 LOG_write((uint8_t)LOG_FATAL, fmt, ##__VA_ARGS__)
#define LOG_ERROR(fmt, ...)                 LOG_write((uint8_t)LOG_ERROR, fmt, ##__VA_ARGS__)
#define LOG_IEC()                           LOG_ERROR("Internal error: %s:%d", __FILE__, __LINE__);
#define LOG_WARN(fmt, ...)                  LOG_write((uint8_t)LOG_WARN, fmt, ##__VA_ARGS__)
#define LOG_INFO(fmt, ...)                  LOG_write((uint8_t)LOG_INFO, fmt, ##__VA_ARGS__)
#define LOG_VERBOSE(v_level, fmt, ...)      LOG_write((uint8_t)LOG_VERBOSE + v_level, fmt, ##__VA_ARGS__)
#define LOG_write(level, fmt, ...)          LOG_print(__FILE__, __LINE__, LOG_CONSOLE_THRESHOLD_THIS, LOG_FILE_THRESHOLD_THIS, level, fmt, ##__VA_ARGS__)

/**
 * @brief Creates an asynchronous print request
 * @return 0 on success, -1 on failure
*/
int8_t LOG_print(const char* file, uint16_t line, int8_t console_threshold, int8_t file_threshold, int8_t level, const char* fmt, ...);

/**
 * @brief Initializes the Log thread for running
 * @return 0 on success, -1 on failure
*/
int8_t LOG_init();

/**
 * @brief Starts the log thread
 * @return 0 on success, -1 on failure
*/
int8_t LOG_start();

/**
 * @brief Stops the log thread. Blocks until all requests are handled
 * @return 0 on success, -1 on failure
*/
int8_t LOG_stop();

/**
 * @brief Releases initialized resources from module
 * @return 0 on success, -1 on failure
*/
int8_t LOG_destory();