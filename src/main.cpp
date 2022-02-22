#include "main.h"

#define LED_GREEN PIN_LED1

// Semaphore used by events to wake up loop task
SemaphoreHandle_t g_task_sem = NULL;

// Timer to wakeup task frequently and send message
SoftwareTimer g_task_wakeup_timer;

// Flag for the event type
volatile uint16_t g_task_event_type = NO_EVENT;

/** Flag showing if TX cycle is ongoing */
bool lora_busy = false;

/** Set the device name, max length is 10 characters */
char lora_packet[10] = "HELLO";

// Period Wakeup for Statuses
void periodic_wakeup(TimerHandle_t unused)
{
    // Switch on LED to show we are awake
    digitalWrite(LED_BUILTIN, HIGH);
    g_task_event_type |= STATUS;
    xSemaphoreGiveFromISR(g_task_sem, pdFALSE);
}

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

    DEBUG_LOG("APP", "===========================");
    DEBUG_LOG("APP", "RAK4631 Firmware Starting");
    DEBUG_LOG("APP", "===========================");

    // Init BLE
    init_ble();

    init_lorawan();

    // LoRa is setup, start the timer that will wakeup the loop frequently
    g_task_wakeup_timer.begin(500, periodic_wakeup);
    g_task_wakeup_timer.start();

    DEBUG_LOG("APP", "Setup Complete");
}

void loop()
{
    // DEBUG_LOG("APP", "Beginning of Loop");
    // Sleep until we are woken up by an event
    if (xSemaphoreTake(g_task_sem, portMAX_DELAY) == pdTRUE)
    {
        // DEBUG_LOG("APP", "In xSemaphoreTake Loop");
        // DEBUG_LOG("APP", "Event Type: %d\n", g_task_event_type);
        // Switch on green LED to show we are awake
        digitalWrite(LED_BUILTIN, HIGH);
        while (g_task_event_type != NO_EVENT)
        {
            // Handle LoRa data events
			lora_data_handler();
            // Timer triggered event, check serial
            if ((g_task_event_type & STATUS) == STATUS)
            {
                g_task_event_type &= N_STATUS;
                uint8_t temp;
                while (Serial.available() > 0)
                {
                    temp = Serial.read();
                    at_serial_input(uint8_t(temp));
                    ble_uart.printf("%c", temp);
                    delay(5);
                }
                // DEBUG_LOG("SERIAL", "Received Command");
                // DEBUG_LOG("APP", "Timer wakeup");
                // if (lora_busy)
                // {
                //     DEBUG_LOG("APP", "LoRaWAN TX cycle not finished, skip this event");
                // }
                // else
                // {
                //     lmh_error_status result;
                //     // result = send_lora_packet((uint8_t *)&lora_packet, 10);

                //     switch (result)
                //     {
                //     case LMH_SUCCESS:
                //         DEBUG_LOG("APP", "Packet enqueued");
                //         /// \todo set a flag that TX cycle is running
                //         lora_busy = true;
                //         break;
                //     case LMH_BUSY:
                //         DEBUG_LOG("APP", "LoRa transceiver is busy");
                //         break;
                //     case LMH_ERROR:
                //         DEBUG_LOG("APP", "Packet error, too big to send with current DR");
                //         break;
                //     }
                // }
            }
            // BLE UART data handling
            if ((g_task_event_type & BLE_DATA) == BLE_DATA)
            {
                /** BLE UART data arrived */
                g_task_event_type &= N_BLE_DATA;
                uint8_t temp;
                while (ble_uart.available() > 0)
                {
                    temp = ble_uart.read();
                    at_serial_input(uint8_t(temp));
                    Serial.printf("%c", temp);
                    delay(5);
                }
                DEBUG_LOG("BLE UART", "Received Command");
            }
        }

        // reset main loop timer
        g_task_wakeup_timer.reset();

        Serial.flush();
        g_task_event_type = 0;
        // Go back to sleep
        xSemaphoreTake(g_task_sem, 10);
        // Switch off blue LED to show we go to sleep
        digitalWrite(LED_BUILTIN, LOW);
        delay(10);
    }
}

/**
 * @brief Handle received LoRa Data
 * 
 */
void lora_data_handler(void)
{
	// LoRa Join finished handling
	if ((g_task_event_type & LORA_JOIN_FIN) == LORA_JOIN_FIN)
	{
		g_task_event_type &= N_LORA_JOIN_FIN;
		if (g_join_result)
		{
			DEBUG_LOG("APP", "Successfully joined network");
		}
		else
		{
			DEBUG_LOG("APP", "Join network failed");
			/// \todo here join could be restarted.
			// lmh_join();
		}
	}

	// LoRa data handling
	if ((g_task_event_type & LORA_DATA) == LORA_DATA)
	{
		/**************************************************************/
		/**************************************************************/
		/// \todo LoRa data arrived
		/// \todo parse them here
		/**************************************************************/
		/**************************************************************/
		g_task_event_type &= N_LORA_DATA;
		DEBUG_LOG("APP", "Received package over LoRa");
		char log_buff[g_rx_data_len * 3] = {0};
		uint8_t log_idx = 0;
		for (int idx = 0; idx < g_rx_data_len; idx++)
		{
			sprintf(&log_buff[log_idx], "%02X ", g_rx_lora_data[idx]);
			log_idx += 3;
		}
		lora_busy = false;
		DEBUG_LOG("APP", "%s", log_buff);
	}

	// LoRa TX finished handling
	if ((g_task_event_type & LORA_TX_FIN) == LORA_TX_FIN)
	{
		g_task_event_type &= N_LORA_TX_FIN;

		DEBUG_LOG("APP", "LPWAN TX cycle %s", g_rx_fin_result ? "finished ACK" : "failed NAK");

		/// \todo reset flag that TX cycle is running
		lora_busy = false;
	}
}