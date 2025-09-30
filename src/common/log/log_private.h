/** Log Module structural information */

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

/** Buffer for Log Entry */
typedef struct _log_buffer
{
    uint8_t flags;
        #define LOG_BUFFER_FLAG_OUT_CONSOLE 0   /** Flag for writing to console */
        #define LOG_BUFFER_FLAG_OUT_FILE 1      /** Flag for writing to file */

    struct timespec timestamp;                  /** Timestamp of when the log message was submitted to the wr_queue */
    int8_t level;                               /** Priority level of message */
    char msg[LOG_MESSAGE_LEN];                  /** Message content */
    uint8_t len;                                /** Length of message in bytes */
    S_List_Node node;                           /** Node for tracking buffer through queues */
} LOG_Buffer;

/** Log Module Configuration */
typedef struct _log_config
{
    bool en;                                    /** Indicates log enabled vs disabled */  
    bool print_loc;                             /** Indicates whether or not to print source file locations */      
    const char *path;                           /** Path of log file */          
} LOG_Config;

/** Log Module Resources */
typedef struct _log
{
    pthread_t ptid;                             /** Pthread ID of Log Module thread */
    pthread_mutex_t gen_lock;                   /** Generic Lock */
    pthread_mutex_t wr_lock;                    /** Lock protecting the wr_queue */
    pthread_mutex_t free_lock;                  /** Lock protecting the free_queue */
    LOG_Config config;                          /** Configuration */
    // LOG_Buffer buffer_pool[LOG_MAX_BUFFER];     /** Statically allocated pool of Log buffers to rotate through */
    uint32_t pool_count;                        /** Number of LOG_Buffers in buffer_pool */
    LOG_Buffer *buffer_pool;                    /** Dynamically allocated pool of Log buffers to rotate through */
    S_List wr_queue;                            /** Queue of Log Buffers waiting to write to console/file */
    S_List free_queue;                          /** Queue of available Log Buffers */
    int fd;                                     /** Log file descriptor */
} Log;

// Private functions

void LOG_thread_poll ();
void LOG_thread_print();