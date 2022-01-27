#include "main.h"

#define LED_GREEN PIN_LED1

// Semaphore used by events to wake up loop task
SemaphoreHandle_t g_task_sem = NULL;

// Timer to wakeup task frequently and send message
SoftwareTimer g_task_wakeup_timer;

// Flag for the event type
volatile uint16_t g_task_event_type = NO_EVENT;

void setup()
{
    // Create the task event semaphore
    g_task_sem = xSemaphoreCreateBinary();
    // Initialize semaphore
    xSemaphoreGive(g_task_sem);

    // Initialize the built in Green LED
    pinMode(LED_GREEN, OUTPUT);
    digitalWrite(LED_GREEN, LOW);

#if MY_DEBUG > 0
    // Initialize Serial for debug output
    Serial.begin(115200);

    time_t serial_timeout = millis();
    // On nRF52840 the USB serial is not available immediately
    while (!Serial)
    {
        if ((millis() - serial_timeout) < 5000)
        {
            delay(100);
            digitalWrite(LED_GREEN, !digitalRead(LED_GREEN));
        }
        else
        {
            break;
        }
    }
#endif

    digitalWrite(LED_GREEN, HIGH);

    MYLOG("APP", "===========================");
    MYLOG("APP", "RAK4631 Firmware Starting");
    MYLOG("APP", "===========================");

    // Init BLE
    init_ble();

    MYLOG("APP", "Setup Complete");
}

void loop()
{
    MYLOG("APP", "Beginning of Loop");
    // Sleep until we are woken up by an event
    if (xSemaphoreTake(g_task_sem, portMAX_DELAY) == pdTRUE)
    {
        MYLOG("APP", "In xSemaphoreTake Loop");
        Serial.printf("Event Type: %d\n", g_task_event_type);
        // Switch on green LED to show we are awake
        // digitalWrite(LED_BUILTIN, HIGH);
        while (g_task_event_type != NO_EVENT)
        {
            // BLE UART data handling
            if ((g_task_event_type & BLE_DATA) == BLE_DATA)
            {
                MYLOG("AT", "RECEIVED BLE");
                /** BLE UART data arrived */
                g_task_event_type &= N_BLE_DATA;

                while (ble_uart.available() > 0)
                {
                    Serial.printf("%c", ble_uart.read());
                    delay(5);
                }
            }
        }

        if (ble_uart_is_connected)
        {
            ble_uart.printf("Finished Receiving\n");
        }

        g_task_event_type = 0;
		// Go back to sleep
		xSemaphoreTake(g_task_sem, 10);
    }
}