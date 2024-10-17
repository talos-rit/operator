#pragma once

#include <stdint.h>

#define UTIL_len(arr) (sizeof(arr)/arr[0])
#define UTIL_modulo(a,b) (((a%b) + b) % b)
#define STRLEN(s) sizeof(s)