#pragma once

#include <pthread.h>
#include <stdint.h>
#include <time.h>
#include <stdbool.h>

#include "data/s_list.h"
#include "log/log.h"

const char* const _LOG_level_color[] =
{
    BRED,   // Fatal
    RED,    // Error
    YEL,    // Warn
    BLU,    // Info
    WHT,    // Verbose
};

#ifdef LOG_LEVEL_ENTRY
#undef LOG_LEVEL_ENTRY
#endif
#define LOG_LEVEL_ENTRY(entry) #entry
    const char* const log_level_names[] = {LOG_LEVEL_ENTRIES};
#undef LOG_LEVEL_ENTRY

typedef struct _log_buffer
{
    uint8_t flags;
        #define LOG_BUFFER_FLAG_OUT_CONSOLE 0
        #define LOG_BUFFER_FLAG_OUT_FILE 1

    struct timespec timestamp;
    int8_t level;
    char msg[LOG_MESSAGE_LEN];
    uint8_t len;
    S_List_Node node;
} LOG_Buffer;

typedef struct _log_config
{
    bool en;
    bool print_loc;
    const char *path;
} LOG_Config;

typedef struct _log
{
    pthread_t ptid;
    pthread_mutex_t gen_lock;
    pthread_mutex_t wr_lock;
    pthread_mutex_t free_lock;

    LOG_Config config;
    LOG_Buffer buffer_pool[LOG_MAX_BUFFER];
    S_List wr_queue;
    S_List free_queue;

    int fd;
} Log;

// Private functions

void LOG_thread_poll ();
void LOG_thread_print();