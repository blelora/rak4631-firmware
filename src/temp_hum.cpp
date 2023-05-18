#include "temp_hum.h"

#define TEMP_HUM_STACK_SIZE (256 * 4)

s_temp_hum temp_hum;

void temp_hum_task(void *arg);

void temp_hum_init()
{
    DEBUG_LOG("TEMP_HUM", "INIT");
    xTaskCreate(temp_hum_task, "TEMP_HUM", TEMP_HUM_STACK_SIZE, NULL, TASK_PRIO_LOW, NULL);
}

void temp_hum_task(void *arg)
{
    DEBUG_LOG("TEMP_HUM", "TASK START");

    if (!rak1901_init())
    {
        DEBUG_LOG("TEMP_HUM", "TASK DELETING - TEMP HUM failed to init");
        vTaskDelete(NULL);
    }

    while (1)
    {
        SHTC3_Status_TypeDef result = mySHTC3.update();
        vTaskDelay(200);
        if (mySHTC3.lastStatus == SHTC3_Status_Nominal) // You can also assess the status of the last command by checking the ".lastStatus" member of the object
        {
            DEBUG_LOG("TEMP_HUM", "TASK LOOP");
            temp_hum.temperature = mySHTC3.toDegF();
            DEBUG_LOG("TEMP_HUM", "Temperature: %f deg F", temp_hum.temperature);

            temp_hum.humidity = mySHTC3.toPercent();
            DEBUG_LOG("TEMP_HUM", "Humidity: %f %", temp_hum.humidity);
        }

        vTaskDelay(30000);
    }
}