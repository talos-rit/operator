#include <stdarg.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <sys/time.h>
#include <string.h>
#include <stddef.h>
#include <unistd.h>
#include <fcntl.h>

#include "log/log.h"
#include "log/log_private.h"
#include "util/comm.h"
#include "util/timestamp.h"

#define LOG_CLOCK CLOCK_REALTIME
#define LOG_TS_LEN (UTIL_TIMESTAMP_LEN + UTIL_TS_MSEC_LEN + 1)

#define LOG_CONSOLE_THRESHOLD_THIS  LOG_THRESHOLD_QUIET
#define LOG_FILE_THRESHOLD_THIS     LOG_THRESHOLD_QUIET

Log the_log;



static int8_t LOG_timestamp(char* dst_str)
{
    struct timespec ts;
    int ret = clock_gettime(LOG_CLOCK, &ts);
    char* str_iter = dst_str;
    
    ret = UTIL_time_iso8601_timestamp_UTC(str_iter, ts.tv_sec);
    if (ret == -1) return -1; // Can't log error: recursive call
    str_iter += ret;
    
    ret = UTIL_time_msec_timestamp(str_iter, ts.tv_nsec / 1000);
    if (ret == -1) return -1; // Can't log error: recursive call
    str_iter += ret;
    str_iter += sprintf(str_iter, "Z");
    return str_iter - dst_str;
}

static uint8_t get_level_name(char* dst, uint8_t level, bool color)
{
    if (level > 9 + (uint8_t) LOG_VERBOSE) return -1;

    int8_t vlev = -1;
    uint8_t offset = 0;

    if (level >= (uint8_t) LOG_VERBOSE)
    { 
        vlev = level - (uint8_t) LOG_VERBOSE;
        level = LOG_VERBOSE;
    }

    if (color)
        offset += sprintf(dst + offset, "%s", _LOG_level_color[level]);

    if (vlev != -1)
        offset += sprintf(dst + offset, "%s:%d ", log_level_names[level], vlev);
    else
        offset += sprintf(dst + offset, "%-9s ", log_level_names[level]);  // Left justified, 13 for "LOG_VERBOSE:0"
    
    if (color)
        offset += sprintf(dst + offset, "%s", COLOR_RESET);

    return offset;
}

static uint16_t format_log_entry(char* dst, LOG_Buffer *buf, bool use_color)
{
    uint16_t msg_iter = 0;

    // Timestamp
    char ts[LOG_TS_LEN];
    LOG_timestamp(&ts[0]);
    msg_iter += sprintf(dst + msg_iter, "[%s] ", ts);

    // Level
    msg_iter += get_level_name(dst + msg_iter, buf->level, use_color);

    // Message
    msg_iter += sprintf(dst + msg_iter, "%s", buf->msg);
    msg_iter += sprintf(dst + msg_iter, "\n");
    return msg_iter;
}

void LOG_thread_print(LOG_Buffer *buf)
{    
    char msg[1024];

    // Console out
    if (buf->flags & (1 << LOG_BUFFER_FLAG_OUT_CONSOLE))
    {
        format_log_entry(&msg[0], buf, LOG_USE_COLOR);
        #if LOG_USE_STDERR
        if (buf->level <= LOG_ERROR)
            fprintf(stderr, "%s", msg);
        else printf("%s", msg);
        #else
        printf("%s", msg);
        #endif
    }

    // File out
    if (buf->flags & (1 << LOG_BUFFER_FLAG_OUT_FILE))
    {
        uint16_t len = format_log_entry(&msg[0], buf, false) + 1;
        write(the_log.fd, &msg[0], len-1);
    }
}

static int8_t LOG_Buffer_init(LOG_Buffer *buf)
{
    if (!buf) STD_FAIL;
    memset(buf, 0, sizeof(LOG_Buffer));
    DATA_S_List_Node_init(&buf->node);
    DATA_S_List_append(&the_log.free_queue, &buf->node);
    return 0;
}

void LOG_thread_poll()
{
    pthread_mutex_lock (&the_log.wr_lock);
    LOG_Buffer *buf = DATA_LIST_GET_OBJ(DATA_S_List_pop(&the_log.wr_queue), LOG_Buffer, node);
    pthread_mutex_unlock (&the_log.wr_lock);

    LOG_thread_print(buf);

    pthread_mutex_lock (&the_log.free_lock);
    LOG_Buffer_init(buf);
    pthread_mutex_unlock (&the_log.free_lock);
}

static LOG_Buffer *get_buffer(Log *log)
{
    pthread_mutex_lock (&log->free_lock);
    LOG_Buffer* buf = DATA_LIST_GET_OBJ(DATA_S_List_pop(&log->free_queue), LOG_Buffer, node);
    pthread_mutex_unlock(&log->free_lock);
    return buf;
}

static int8_t push_buffer(Log *log, LOG_Buffer *buf)
{
    if (!log || !buf) STD_FAIL;
    pthread_mutex_lock (&log->wr_lock);
    DATA_S_List_append(&log->wr_queue, &buf->node);
    pthread_mutex_unlock(&log->wr_lock);
    return 0;
}

int8_t LOG_print(const char* file, uint16_t line, int8_t console_threshold, int8_t file_threshold, int8_t level, const char* fmt, ...)
{
    if (!the_log.config.en) return -1;
    uint8_t flags = 0;
    if (level <= console_threshold) flags |= (1 << LOG_BUFFER_FLAG_OUT_CONSOLE);
    if (level <= file_threshold)    flags |= (1 << LOG_BUFFER_FLAG_OUT_FILE);
    if ((flags & (  (1 << LOG_BUFFER_FLAG_OUT_CONSOLE) | 
                    (1 << LOG_BUFFER_FLAG_OUT_FILE))) == 0) return 0;

    struct timespec ts;
    int ret = clock_gettime(LOG_CLOCK, &ts);
    if (ret == -1) STD_FAIL;

    LOG_Buffer *buf = get_buffer(&the_log);
    buf->timestamp = ts;
    buf->level = level;
    buf->flags = flags;

    va_list list;
    va_start(list, fmt);
    if (the_log.config.print_loc)
    {
        buf->len += sprintf(&buf->msg[buf->len], "[%s : %4d]: ", file, line);
    }
    buf->len += vsprintf(&buf->msg[buf->len], fmt, list) + 1;
    va_end(list);

    ret = push_buffer(&the_log, buf);
    if (ret == -1) STD_FAIL;

    return 0;
}

static void *LOG_run(void*)
{
    // TODO: add exit polling
    while(1)
    {
        if (the_log.wr_queue.len == 0) 
        {
            if (!the_log.config.en) break;
            usleep(25*1000);
            continue;
        }
        LOG_thread_poll();
    }

    return 0;
}

static int8_t init_log_lists(Log *log)
{
    if (!log) STD_FAIL
    
    DATA_S_List_init (&log->free_queue);
    DATA_S_List_init (&log->wr_queue);

    for (uint16_t i = 0; i < LOG_MAX_BUFFER; i++)
    {
        if (LOG_Buffer_init(&log->buffer_pool[i]) == -1) STD_FAIL
    }

    return 0;
}

int8_t LOG_init()
{
    memset(&the_log, 0, sizeof(Log));
    if (-1 == pthread_mutex_init(&the_log.gen_lock,  NULL))   STD_FAIL
    if (-1 == pthread_mutex_init(&the_log.wr_lock,   NULL))   STD_FAIL
    if (-1 == pthread_mutex_init(&the_log.free_lock, NULL))   STD_FAIL
    if (-1 == init_log_lists(&the_log)) STD_FAIL
    the_log.config.print_loc = LOG_USE_LOCATION;
    the_log.config.path = LOG_FILE_PATH;
    the_log.fd = open(the_log.config.path, O_CREAT | O_WRONLY | O_TRUNC | O_APPEND, 0666);

    LOG_INFO("Log Module Initialized.");
    return 0;
}

int8_t LOG_start()
{
    pthread_create(&the_log.ptid, NULL, LOG_run, NULL);
    the_log.config.en = true;
    LOG_INFO("Log Module Running...");
    return -1;
}

int8_t LOG_stop()
{
    the_log.config.en = false; // Prevent new log statements
    pthread_join(the_log.ptid, NULL);
    return 0;
}

int8_t LOG_destory()
{
    close(the_log.fd);
    pthread_mutex_destroy(&the_log.gen_lock);
    return -1;
}