#include "accel.h"

#define ACCEL_STACK_SIZE (256 * 4)

// s_accel accel;

#define CLICKTHRESHHOLD 40

volatile bool z_high = false;
volatile bool z_low = false;


volatile bool flipped = false;
int flipCount = 0;
void flipDetected(void);

void accel_task(void *arg);

void accel_init()
{
    DEBUG_LOG("ACCEL", "INIT");
    xTaskCreate(accel_task, "ACCEL", ACCEL_STACK_SIZE, NULL, TASK_PRIO_LOW, NULL);
}

void accel_task(void *arg)
{
    DEBUG_LOG("ACCEL", "TASK START");

    if (!rak1904_init())
    {
        DEBUG_LOG("ACCEL", "TASK DELETING - ACCEL failed to init");
        vTaskDelete(NULL);
    }

    while (1)
    {
        // DEBUG_LOG("ACCEL", "TASK LOOP");

        // uint8_t orient = lis.getOrientation();
        // if (orient != 0) 
        // {
        //     if (! (orient & 0x30)) return;
        //     DEBUG_LOG("ACCEL", "orient detected (0x%x)", orient);
        //     if (orient & 0x20) 
        //     {
        //         if(!z_high)
        //         {
        //             z_high = true;
        //             z_low = false;
        //             flipDetected();
        //             DEBUG_LOG("ACCEL", "Z HIGH");
        //         }
        //     }
        //     if (orient & 0x10)
        //     {
        //         if(!z_low) 
        //         {
        //             z_high = false;
        //             z_low = true;
        //             flipDetected();
        //             DEBUG_LOG("ACCEL", "Z LOW");
        //         }
        //     }
        //     if (flipped) {
        //         flipCount++;
        //         DEBUG_LOG("ACCEL", "Flip detected! Total flips: %d", flipCount);
        //         if(flipCount % 2 == 0)
        //         {
        //             DEBUG_LOG("ACCEL", "Rotations : %d", flipCount/2);
        //         }
        //         flipped = false;
        //     }
        // }

        // lis.read();      // get X Y and Z data at once
        // // Then print out the raw data
        // DEBUG_LOG("ACCEL", "START *****************************");
        // DEBUG_LOG("ACCEL", "X: %d", lis.x);
        // DEBUG_LOG("ACCEL", "Y: %d", lis.y);
        // DEBUG_LOG("ACCEL", "Z: %d", lis.z);

        /* Or....get a new sensor event, normalized */
        sensors_event_t event;
        lis.getEvent(&event);

        /* Display the results (acceleration is measured in m/s^2) */
        DEBUG_LOG("ACCEL", "X: %f", event.acceleration.x);
        DEBUG_LOG("ACCEL", "Y: %f", event.acceleration.y);
        DEBUG_LOG("ACCEL", "Z: %f", event.acceleration.z);
        // DEBUG_LOG("ACCEL", "END *****************************");

        vTaskDelay(50);
    }
}

void flipDetected() {
  flipped = true;
}