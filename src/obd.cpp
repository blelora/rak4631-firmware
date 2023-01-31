#include "obd.h"

#define OBD_STACK_SIZE (256 * 4)

void obd_task(void *arg);

s_obd_data obd_data;
lorawan_payload_s obd_payload;

void obd_init()
{
    DEBUG_LOG("OBD", "INIT");
    xTaskCreate(obd_task, "OBD", OBD_STACK_SIZE, NULL, TASK_PRIO_LOW, NULL);
}

void obd_task(void *arg)
{
    DEBUG_LOG("OBD", "TASK START");

    if (!serial_can_init())
    {
        DEBUG_LOG("OBD", "TASK DELETING - obd failed to init");
        vTaskDelete(NULL);
    }

    while(1)
    {
        DEBUG_LOG("OBD", "TASK LOOP");

        // int __speed = 0;
        int ret = getSpeed(&obd_data.vehicle_speed);
        if(ret)
        {
            Serial.print("Vehicle Speed: ");
            Serial.print(obd_data.vehicle_speed);
            Serial.println(" kmh");
        }
        vTaskDelay(500);

        // int __rpm = 0;
        ret = getRPM(&obd_data.vehicle_rpm);
        if (ret)
        {
            Serial.print("Engine Speed: ");
            Serial.print(obd_data.vehicle_rpm);
            Serial.println(" rpm");
        }
        vTaskDelay(500);

        // unsigned char vin_buf[17] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
        ret = getVIN(obd_data.vin);
        if (ret)
        {
            Serial.print("VIN: ");
            for (int i = 0; i < 17; i++)
            {
            // Serial.println(vin_buf[i], HEX);
            Serial.write(obd_data.vin[i]);
            }
            Serial.println("\n");
        }
        else{
            Serial.println("\nCould not get VIN");
        }

        obd_payload.data_length = sizeof(obd_data);
        obd_payload.fport = 3;
        memcpy(obd_payload.data, &obd_data, sizeof(obd_data));

        xQueueSend(xStructQueue,( void * ) &obd_payload,( TickType_t ) 0 );

        vTaskDelay(5000);
    }
}