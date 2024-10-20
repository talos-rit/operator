/**
 * Timestamping Module
 * Used for converting time_t structs into ISO8601 timestamps
 * Supports local timezones, UTC/Zulu timezone, and optionally milliseconds
*/

#pragma once

#include <stdint.h>
#include <time.h>
#include <sys/time.h>

#ifdef __cplusplus
extern "C" {
#endif

// ISO 6801
#define UTIL_DATESTAMP              "%F"
                                //  YYYY-MM-DD
#define UTIL_DATESTAMP_LEN          (10+1)

#define UTIL_TIMESTAMP              "T%T"
                                //  THH:MM:SS
#define UTIL_TIMESTAMP_LEN          (1+(2*3)+2+1)

#define UTIL_TS_OFFSET              "%+02d:%02d"
                                //   +TH:TS
#define UTIL_TS_OFFSET_LEN          (1+2+1+2+1)       

#define UTIL_TS_MSEC                ":%03d" // Also used for usec
                                //  :mmm
#define UTIL_TS_MSEC_LEN            (1+3+1)
// #define UTIL_TS_USEC                UTIL_TIMESTAMP_MSEC UTIL_TIMESTAMP_MSEC
// #define UTIL_TS_USEC_LEN            (2 * UTIL_TIMESTAMP_MSEC_LEN)

// #define UTIL_TS_ISO6801_Z               UTIL_TIMESTAMP         "Z"
// #define UTIL_TS_ISO6801_Z_LEN          (UTIL_TIMESTAMP_LEN +   1)
// #define UTIL_TS_ISO6801                 UTIL_TIMESTAMP         UTIL_OFFSET
// #define UTIL_TS_ISO6801_LEN            (UTIL_TIMESTAMP_LEN +   UTIL_OFFSET_LEN)
// #define UTIL_TS_ISO6801_MSEC            UTIL_TS_ISO6801            UTIL_TS_MSEC
// #define UTIL_TS_ISO6801_LEN            (UTIL_TS_ISO6801_LEN +      UTIL_TS_MSEC_LEN)
// #define UTIL_TS_ISO6801_USEC            UTIL_TS_ISO6801_MSEC       UTIL_TS_MSEC
// #define UTIL_TS_ISO6801_LEN            (UTIL_TS_ISO6801_MSEC_LEN + UTIL_TS_MSEC_LEN)

// #define UTIL_DTS_ISO6801_Z               UTIL_DATESTAMP         UTIL_TS_ISO6801_Z
// #define UTIL_DTS_ISO6801_Z_LEN          (UTIL_DATESTAMP_LEN +   UTIL_TS_ISO6801_Z_LEN)
// #define UTIL_DTS_ISO6801                 UTIL_DATESTAMP         UTIL_TS_ISO6801
// #define UTIL_DTS_ISO6801_LEN            (UTIL_DATESTAMP_LEN +   UTIL_TS_ISO6801_LEN)
// #define UTIL_DTS_ISO6801_MSEC            UTIL_DATESTAMP         UTIL_TS_ISO6801_MSEC
// #define UTIL_DTS_ISO6801_LEN            (UTIL_DATESTAMP_LEN +   UTIL_TS_ISO6801_LEN)
// #define UTIL_DTS_ISO6801_USEC            UTIL_DATESTAMP         UTIL_TS_ISO6801_USEC
// #define UTIL_DTS_ISO6801_LEN            (UTIL_DATESTAMP_LEN +   UTIL_TS_ISO6801_LEN)



/**
 * @brief Formats an ISO6801 YYYY-MM-DD datestamp at the string provided
 * 
 * @param dst_str pointer to string to be formatted
 * 
 * @param time time to be stamped
 * 
 * @return the length of the formatted string upon success, -1 upon failure
*/
int8_t UTIL_time_iso8601_datestamp_UTC(char* dst_str, time_t time);

/**
 * @brief Formats an ISO6801 YYYY-MM-DD datestamp at the string provided
 * 
 * @param dst_str pointer to string to be formatted
 * 
 * @param time time to be stamped
 * 
 * @return the length of the formatted string upon success, -1 upon failure
*/
int8_t UTIL_time_iso8601_datestamp_local(char* dst_str, time_t time);

/**
 * @brief Formats an ISO6801 THH:MM:SSZ timestamp at the string provided (UTC/Zulu time)
 * 
 * @param dst_str pointer to string to be formatted
 * 
 * @param time time to be stamped
 * 
 * @return the length of the formatted string upon success, -1 upon failure
*/
int8_t UTIL_time_iso8601_timestamp_UTC(char* dst_str, time_t time);

/**
 * @brief Formats an ISO6801 THH:MM:SS timestamp at the string provided (local time)
 * 
 * @param dst_str pointer to string to be formatted
 * 
 * @param time time to be stamped
 * 
 * @return the length of the formatted string upon success, -1 upon failure
*/
int8_t UTIL_time_iso8601_timestamp_local(char* dst_str, time_t time);

/**
 * @brief Formats the millesecond portion of a timestamp at the string provided
 * 
 * @param dst_str pointer to string to be formatted
 * 
 * @param time time to be stamped
 * 
 * @return the length of the formatted string upon success, -1 upon failure
*/
int8_t UTIL_time_msec_timestamp(char* dst_str, __useconds_t time);

/**
 * @brief Formats the milli- and micro- second portion of a timestamp at the string provided
 * 
 * @param dst_str pointer to string to be formatted
 * 
 * @param time time to be stamped
 * 
 * @return the length of the formatted string upon success, -1 upon failure
*/
int8_t UTIL_time_msec_usec_timestamp(char* dst_str, __useconds_t time);

/**
 * @brief Formats a ISO6801 timezone offset (+TH:TM) at the string provided
 * 
 * @param dst_str pointer to string to be formatted
 * 
 * @param tz_offset_sec timezone offset in minutes compared to UTC (a value of zero will result in a "Z", like UTC)
 * 
 * @return the length of the formatted string upon success, -1 upon failure
*/
int8_t UTIL_time_iso8601_time_offset(char* dst_str, int16_t tz_offset_sec);

/**
 * @brief Formats a complete ISO6801 date-time stamp, with offset (YYYY-MM-DDTHH:MM:SS+TH:TM) at the string provided
 * 
 * @param dst_str pointer to string to be formatted
 * 
 * @param time time to be stamped
 * 
 * @param tz_offset_min timezone offset in minutes compared to UTC (a value of zero will result in a "Z", like UTC)
 * 
 * @return the length of the formatted string upon success, -1 upon failure
*/
int8_t UTIL_time_iso8601_complete_datetime(char* dst_str, time_t time, int16_t tz_offset_min);

#ifdef __cplusplus
}
#endif