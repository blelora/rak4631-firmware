#include "main.h"

#ifndef __AT_H__
#define __AT_H__

#define AT_ERROR "+CME ERROR:"
#define ATCMD_SIZE 160
#define ATQUERY_SIZE 128

#define AT_ERRNO_NOSUPP (1)
#define AT_ERRNO_NOALLOW (2)
#define AT_ERRNO_PARA_VAL (5)
#define AT_ERRNO_PARA_NUM (6)
#define AT_ERRNO_EXEC_FAIL (7)
#define AT_ERRNO_SYS (8)
#define AT_CB_PRINT (0xFF)

#endif