/**
 *  A file containing useful values/shortcuts
 */

#pragma once
#include "log/log.h"

#define STD_FAIL \
  {              \
    LOG_IEC();   \
    return -1;   \
  }  // Not quite sure where to put this
#define STD_FAIL_VOID \
  {                   \
    LOG_IEC();        \
    return;           \
  }  // Not quite sure where to put this
#define STD_FAIL_VOID_PTR \
  {                       \
    LOG_IEC();            \
    return NULL;          \
  }  // Not quite sure where to put this
