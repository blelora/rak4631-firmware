#include "main.h"
#include "battery.h"
#include "at_cmd.h"
#include "lorawan.h"
#include "gnss.h"
#include "temp_hum.h"

#include <Wire.h>

#define SYSTEM_STACK_SIZE (256 * 4)

/* System wide synchronization */
xSemaphoreHandle canStartMutex;

void system_task(void *arg);

void system_init(void)
{
    DEBUG_LOG("SYSTEM", "INIT");
    // Start System Task
    xTaskCreate(system_task, "SYSTEM", SYSTEM_STACK_SIZE, NULL, TASK_PRIO_LOW, NULL);
}

void system_bringup(void)
{
    DEBUG_LOG("SYSTEM", "BRINGUP");

    canStartMutex = xSemaphoreCreateMutex();
	xSemaphoreTake(canStartMutex, portMAX_DELAY);

    Wire.begin();

    init_flash();
    battery_init();
    at_cmd_init();
    // gnss_init();
    temp_hum_init();
    init_ble();
    lorawan_init();
}

void system_task(void *arg)
{
    DEBUG_LOG("SYSTEM", "TASK START");
    system_bringup();
    while (1)
    {
        // DEBUG_LOG("SYSTEM", "TASK LOOP");
        vTaskDelay(1000);
    }
}
