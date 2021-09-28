#ifndef __PRINT_BACKTRACE_H__
#define __PRINT_BACKTRACE_H__

#ifdef HAS_BACKTRACE

#include "internal.h"

#define MAX_BT_LEN (100)

void print_backtrace(void);

#endif

#endif
