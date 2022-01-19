#ifndef MAIN_H
#define MAIN_H

// Debug output, 0 to disable, 1 to enable
#define MY_DEBUG 1

#if MY_DEBUG > 0
#define MYLOG(tag, ...)           \
    do                            \
    {                             \
        if (tag)                  \
            PRINTF("[%s] ", tag); \
        PRINTF(__VA_ARGS__);      \
        PRINTF("\n\r");           \
    } while (0)
#else
#define MYLOG(...)
#endif

#include <Arduino.h>
#include <nrf_nvic.h>

#include "lorawan.h"

#endif // MAIN_H