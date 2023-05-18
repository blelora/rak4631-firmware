#include "light.h"

#define OPT3001_ADDRESS 0x44

#define LIGHT_STACK_SIZE (256 * 4)

uint32_t light_lux;

void light_task(void *arg);

void light_init()
{
    DEBUG_LOG("LIGHT", "INIT");
    xTaskCreate(light_task, "LIGHT", LIGHT_STACK_SIZE, NULL, TASK_PRIO_LOW, NULL);
}

void light_task(void *arg)
{
    DEBUG_LOG("LIGHT", "TASK START");

    if (!rak1903_init())
    {
        DEBUG_LOG("LIGHT", "TASK DELETING - LIGHT failed to init");
        vTaskDelete(NULL);
    }

    while (1)
    {
        DEBUG_LOG("LIGHT", "TASK LOOP");
        OPT3001 result = opt3001.readResult();
        if (result.error == NO_ERROR)
        {
            light_lux = result.lux;
            DEBUG_LOG("LIGHT", "ALV: %d lux", light_lux);
        }
        else
        {
            DEBUG_LOG("LIGHT", "Error: %d", result.error);
        }
        vTaskDelay(30000);
    }
}