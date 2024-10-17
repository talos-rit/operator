#include "util/timestamp.h"

#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <sys/time.h>

#include "log/log.h"
#include "util/array.h"

//NOTE: can't use debug logs: logs use timestamps

int8_t UTIL_time_iso8601_datestamp_UTC(char* dst_str, time_t time)
{
    if (!dst_str)
    {
        return -1;
    }

    struct tm *ftime = gmtime(&time);
    size_t ret = strftime(dst_str, UTIL_DATESTAMP_LEN, UTIL_DATESTAMP, ftime);
    return ret == 0 ? (size_t) -1 : ret;
}

int8_t UTIL_time_iso8601_datestamp_local(char* dst_str, time_t time)
{
    if (!dst_str)
    {
        return -1;
    }

    struct tm *ftime = localtime(&time);
    size_t ret = strftime(dst_str, UTIL_DATESTAMP_LEN, UTIL_DATESTAMP, ftime);
    return ret == 0 ? (size_t) -1 : ret;
}

int8_t UTIL_time_iso8601_timestamp_UTC(char* dst_str, time_t time)
{
    if (!dst_str)
    {
        return -1;
    }

    struct tm *ftime = gmtime(&time);
    size_t ret = strftime(dst_str, UTIL_TIMESTAMP_LEN, UTIL_TIMESTAMP, ftime);
    return ret == 0 ? (size_t) -1 : ret;
}
int8_t UTIL_time_iso8601_timestamp_local(char* dst_str, time_t time)
{
    if (!dst_str)
    {
        return -1;
    }

    struct tm *ftime = localtime(&time);
    size_t ret = strftime(dst_str, UTIL_TIMESTAMP_LEN, UTIL_TIMESTAMP, ftime);
    return ret == 0 ? (size_t) -1 : ret;
}

int8_t UTIL_time_msec_timestamp(char* dst_str, __useconds_t usec)
{
    if (!dst_str)
    {
        return -1;
    }

    size_t ret = snprintf(dst_str, UTIL_TS_MSEC_LEN, UTIL_TS_MSEC, (usec / 1000) % 1000);
    return ret == 0 ? (size_t) -1 : ret;
}

int8_t UTIL_time_msec_usec_timestamp(char* dst_str, __useconds_t usec)
{
    if (!dst_str)
    {
        return -1;
    }

    size_t ret = snprintf(dst_str, UTIL_TS_MSEC_LEN * 2, UTIL_TS_MSEC UTIL_TS_MSEC, (usec / 1000) % 1000, usec % 1000);
    return ret == 0 ? (size_t) -1 : ret;
}

int8_t UTIL_time_iso8601_time_offset(char* dst_str, int16_t tz_offset_sec)
{
    if (!dst_str)
    {
        return -1;
    }

    tz_offset_sec %= 24 * 60 * 60; // Wrap offsets greater than 24 hours
    size_t ret = snprintf(dst_str, UTIL_TS_OFFSET_LEN, UTIL_TS_OFFSET, tz_offset_sec / (60 * 60), (tz_offset_sec / 60) % 60);
    return ret == 0 ? (size_t) -1 : ret;
}

static int8_t iso8601_complete_datetime_UTC(char* dst_str, time_t time)
{
    if (!dst_str)
    {
        return -1;
    }

    char* str_iter = dst_str;
    size_t ret;
    
    ret = UTIL_time_iso8601_datestamp_UTC(str_iter, time);
    if (ret == 0)
    {
        return -1;
    }
    str_iter += ret;

    ret = UTIL_time_iso8601_timestamp_UTC(str_iter, time);
    if (ret == 0)
    {
        return -1;
    }
    str_iter += ret;
    return str_iter - dst_str;
}

static int8_t iso8601_complete_datetime_local(char* dst_str, time_t time)
{
    if (!dst_str)
    {
        return -1;
    }

    char* str_iter = dst_str;
    int8_t ret;
    
    ret = UTIL_time_iso8601_datestamp_local(str_iter, time);
    if (ret == -1)
    {
        return -1;
    }
    str_iter += ret;

    ret = UTIL_time_iso8601_timestamp_local(str_iter, time);
    if (ret == -1)
    {
        return -1;
    }
    str_iter += ret;
    return str_iter - dst_str;
}

int8_t UTIL_time_iso8601_complete_datetime_UTC(char* dst_str, time_t time)
{
    char* str_iter = dst_str;
    int8_t ret = iso8601_complete_datetime_UTC(str_iter, time);
    if (ret == -1)
    {
        return -1;
    }
    str_iter += ret;

    str_iter += sprintf(str_iter, "Z");
    return str_iter - dst_str;
}

int8_t UTIL_time_iso8601_complete_datetime_local(char* dst_str, time_t time)
{
    char* str_iter = dst_str;
    int8_t ret = iso8601_complete_datetime_local(str_iter, time);
    if (ret == -1)
    {
        return -1;
    }
    str_iter += ret;

    struct tm *ftime = localtime(&time);
    str_iter += UTIL_time_iso8601_time_offset(str_iter, ftime->tm_gmtoff);
    return str_iter - dst_str;
}

int8_t UTIL_time_iso8601_complete_datetime_msec_UTC(char* dst_str, time_t time)
{
    char* str_iter = dst_str;
    int8_t ret = iso8601_complete_datetime_UTC(str_iter, time);
    if (ret == -1)
    {
        return -1;
    }
    str_iter += ret;

    ret = UTIL_time_msec_timestamp(str_iter, time);
    if (ret == -1)
    {
        return -1;
    }
    str_iter += ret;

    str_iter += sprintf(str_iter, "Z");
    return str_iter - dst_str;
}

int8_t UTIL_time_iso8601_complete_datetime_msec_local(char* dst_str, time_t time)
{
    char* str_iter = dst_str;
    int8_t ret = iso8601_complete_datetime_local(str_iter, time);
    if (ret == -1)
    {
        return -1;
    }
    str_iter += ret;

    ret = UTIL_time_msec_timestamp(str_iter, time);
    if (ret == -1)
    {
        return -1;
    }
    str_iter += ret;

    struct tm *ftime = localtime(&time);
    str_iter += UTIL_time_iso8601_time_offset(str_iter, ftime->tm_gmtoff);
    return str_iter - dst_str;
}