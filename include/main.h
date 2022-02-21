#ifndef MAIN_H
#define MAIN_H

// Debug output, 0 to disable, 1 to enable
#define DEBUG 1

#if DEBUG > 0
#define DEBUG_LOG(tag, ...)                      \
do                                               \
    {                                            \
        if (tag)                                 \
            PRINTF("[%s] ", tag);                \
        PRINTF(__VA_ARGS__);                     \
        PRINTF("\n\r");                          \
        if (ble_uart_is_connected)               \
            if (tag)                             \
                ble_uart.printf("[%s] ", tag);   \
            ble_uart.printf(__VA_ARGS__);        \
            ble_uart.printf("\n\r");             \
    } while (0)
#else
#define DEBUG_LOG(...)
#endif

#include <Arduino.h>
#include <nrf_nvic.h>

#include "lorawan.h"
#include "ble_core.h"

// Wake up events
#define NO_EVENT 0
#define STATUS 0b0000000000000001
#define N_STATUS 0b1111111111111110
#define BLE_DATA 0b0000000000000100
#define N_BLE_DATA 0b1111111111111011

extern SemaphoreHandle_t g_task_sem;
extern volatile uint16_t g_task_event_type;
extern SoftwareTimer g_task_wakeup_timer;

#endif // MAIN_H