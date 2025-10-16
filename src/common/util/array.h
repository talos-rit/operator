/**
 * Array helper functions
 */

#pragma once

#include <stdint.h>
#include <stdio.h>

#include "util/comm.h"

#define UTIL_len(arr) (sizeof(arr) / sizeof(arr[0]))
#define UTIL_modulo(a, b) (((a % b) + b) % b)
#define STRLEN(s) sizeof(s)
#define UTIL_BYTE_FMT "0x%02X, "
#define UTIL_BYTE_FMT_LEN 6

/**
 * @brief Returns the length of a formated byte-string
 * @param len Length of byte source
 * @returns Length of C String requried to hold formated byte array on success,
 * -1 on failure
 */
#define UTIL_BYTE_STR_FMT_LEN(len) (len * UTIL_BYTE_FMT_LEN + 1)

/**
 * @brief Formats the contents of a byte array into a readable C String
 * @param dst Pointer to String destination
 * @param src Pointer to byte source
 * @param len Length of byte source
 * @returns 0 on success, -1 on failure
 */
inline int UTIL_format_byte_str(char *dst, const uint8_t *src, uint16_t len) {
  if (!dst || !src)
    return -1;

  sprintf(dst, "INIT");
  uint16_t str_iter = 0;
  for (uint16_t iter = 0; iter < len; iter++)
    str_iter += sprintf(&dst[str_iter], UTIL_BYTE_FMT, src[iter]);

  return 0;
}