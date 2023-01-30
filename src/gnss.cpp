#include "gnss.h"

#define GNSS_STACK_SIZE (256 * 4)

void gnss_task(void *arg);

s_gnss_location gnss_location;
lorawan_payload_s gnss_payload;

void gnss_init()
{
    DEBUG_LOG("GNSS", "INIT");
    xTaskCreate(gnss_task, "GNSS", GNSS_STACK_SIZE, NULL, TASK_PRIO_LOW, NULL);
}

void gnss_task(void *arg)
{
    DEBUG_LOG("GNSS", "TASK START");

    if (!rak12500_init())
    {
        DEBUG_LOG("GNSS", "TASK DELETING - GNSS failed to init");
        vTaskDelete(NULL);
    }

    while(1)
    {
        DEBUG_LOG("GNSS", "TASK LOOP");

        gnss_location.accuracy = g_myGNSS.getSIV(200);
        DEBUG_LOG("GNSS","SIV: %d", gnss_location.accuracy);
        if(gnss_location.accuracy >= 6)
        {
            gnss_location.latitude = g_myGNSS.getLatitude(200);
            DEBUG_LOG("GNSS", "Lat: %d", gnss_location.latitude);

            gnss_location.longitude = g_myGNSS.getLongitude(200);
            DEBUG_LOG("GNSS","Long: %d (degrees * 10^-7)", gnss_location.longitude);

            // gnss_location.speed = g_myGNSS.getGroundSpeed(200);
            // DEBUG_LOG("GNSS","Speed: %d (mm/s)", gnss_location.speed);

            // gnss_location.heading = g_myGNSS.getHeading(200);
            // DEBUG_LOG("GNSS","Heading: %d (degrees * 10^-5)", gnss_location.heading);

            gnss_payload.data_length = sizeof(gnss_location);
            memcpy(gnss_payload.data, &gnss_location, sizeof(gnss_location));

            xQueueSend(xStructQueue,( void * ) &gnss_payload,( TickType_t ) 0 );
        }
        
        vTaskDelay(20000);
    }
}