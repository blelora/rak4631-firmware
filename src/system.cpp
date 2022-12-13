#include "main.h"
#include "battery.h"
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
    
    battery_init();
    // at_cmd_init();
    // lorawan_init();
    // init_ble();
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
