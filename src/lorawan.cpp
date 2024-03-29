#include "main.h"
#include "battery.h"
#include "lorawan.h"

#define LORAWAN_STACK_SIZE (256 * 4)

void lorawan_task(void *arg);
int8_t init_lorawan(void);
lmh_error_status send_lora_packet(uint8_t *data, uint8_t size, uint8_t fport);
void lora_data_handler(void);

// Semaphore used by events to wake up loop task
SemaphoreHandle_t g_task_sem = NULL;

/** Flag showing if TX cycle is ongoing */
bool lora_busy = false;

/** Set the device name, max length is 10 characters */
// char lora_packet[6] = "HELLO";
char lora_packet[1] = {0};

// Flag for the event type
volatile uint16_t g_task_event_type = NO_EVENT;

// Timer to wakeup task frequently and send message
SoftwareTimer g_task_lora_tx_wakeup_timer;

/** LoRaWAN credentials from flash */
s_lorawan_credentials g_lorawan_credentials;

/** LoRaWAN setting from flash */
s_lorawan_settings g_lorawan_settings;

/** LoRaWAN setting from flash */
s_lorawan_static_settings g_lorawan_static_settings;

/** Buffer for received LoRaWan data */
uint8_t g_rx_lora_data[256];
/** Length of received data */
uint8_t g_rx_data_len = 0;
/** Buffer for received LoRaWan data */
uint8_t g_tx_lora_data[256];
/** Length of received data */
uint8_t g_tx_data_len = 0;

/** RSSI of last received packet */
int16_t g_last_rssi = 0;
/** SNR of last received packet */
int8_t g_last_snr = 0;
/** fPort of last received packet */
uint8_t g_last_fport = 0;

/** Flag if LoRaWAN is initialized and started */
bool g_lorawan_initialized = false;
/** Result of last TX */
bool g_rx_fin_result;
/** Result of join request */
bool g_join_result = false;

/**************************************************************/
/* LoRaWAN properties                                            */
/**************************************************************/
/** LoRaWAN application data buffer. */
static uint8_t m_lora_app_data_buffer[256];
/** Lora application data structure. */
static lmh_app_data_t m_lora_app_data = {m_lora_app_data_buffer, 0, 0, 0, 0};

// LoRaWAN event handlers
/** LoRaWAN callback when join network finished */
static void lpwan_joined_handler(void);
/** LoRaWAN callback when join network failed */
static void lpwan_join_fail_handler(void);
/** LoRaWAN callback when data arrived */
static void lpwan_rx_handler(lmh_app_data_t *app_data);
/** LoRaWAN callback after class change request finished */
static void lpwan_class_confirm_handler(DeviceClass_t Class);
/** LoRaWAN callback after class change request finished */
static void lpwan_unconfirm_tx_finished(void);
/** LoRaWAN callback after class change request finished */
static void lpwan_confirm_tx_finished(bool result);
/** LoRaWAN Function to send a package */
bool send_lpwan_packet(void);
uint8_t get_lora_batt(void);

/**@brief Structure containing LoRaWAN parameters, needed for lmh_init()
 *
 * Set structure members to
 * LORAWAN_ADR_ON or LORAWAN_ADR_OFF to enable or disable adaptive data rate
 * LORAWAN_DEFAULT_DATARATE OR DR_0 ... DR_5 for default data rate or specific data rate selection
 * LORAWAN_PUBLIC_NETWORK or LORAWAN_PRIVATE_NETWORK to select the use of a public or private network
 * JOINREQ_NBTRIALS or a specific number to set the number of trials to join the network
 * LORAWAN_DEFAULT_TX_POWER or a specific number to set the TX power used
 * LORAWAN_DUTYCYCLE_ON or LORAWAN_DUTYCYCLE_OFF to enable or disable duty cycles
 *                   Please note that ETSI mandates duty cycled transmissions.
 */
static lmh_param_t lora_param_init;

/** Structure containing LoRaWan callback functions, needed for lmh_init() */
// static lmh_callback_t lora_callbacks = {lpwan_joined_handler, lpwan_join_fail_handler};
static lmh_callback_t lora_callbacks = {get_lora_batt, BoardGetUniqueId, BoardGetRandomSeed, lpwan_rx_handler,
                                        lpwan_joined_handler, lpwan_class_confirm_handler, lpwan_join_fail_handler,
                                        lpwan_unconfirm_tx_finished, lpwan_confirm_tx_finished};

bool g_lpwan_has_joined = false;

uint32_t otaaDevAddr = 0;

// Period Wakeup for LoRaWAN TX
void lora_tx_wakeup(TimerHandle_t unused)
{
    // wake up loop for lorawan tx
    digitalWrite(LED_BUILTIN, HIGH);
    g_task_event_type |= LORA_TX;
    xSemaphoreGiveFromISR(g_task_sem, pdFALSE);
}


void lorawan_init()
{
    DEBUG_LOG("LORAWAN", "INIT");
    // Create the task event semaphore
    g_task_sem = xSemaphoreCreateBinary();
    // Initialize semaphore
    xSemaphoreGive(g_task_sem);

    xTaskCreate(lorawan_task, "LORAWAN", LORAWAN_STACK_SIZE, NULL, TASK_PRIO_LOW, NULL);
}

void lorawan_task(void *arg)
{
    DEBUG_LOG("LORAWAN", "TASK START");

    init_lorawan();

    while(1)
    {
        DEBUG_LOG("LORAWAN", "TASK LOOP");
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
                if ((g_task_event_type & LORA_TX) == LORA_TX)
                {
                    g_task_event_type &= N_LORA_TX;
                    DEBUG_LOG("APP", "LORA TX WAKEUP");
                    if (lora_busy)
                    {
                        DEBUG_LOG("APP", "LoRaWAN TX cycle not finished, skip this event");
                    }
                    else
                    {
                        lmh_error_status result;
                        // lora_packet[0] = get_lora_batt();
                        // DEBUG_LOG("LORAWAN BATTERY", "Battery Level Value %d", lora_packet[0]);
                        result = send_lora_packet((uint8_t *)&lora_packet, sizeof(lora_packet), 1);
                        // result = send_lora_packet((uint8_t *)&gnss_location, sizeof(s_gnss_location), 1);

                        switch (result)
                        {
                        case LMH_SUCCESS:
                            DEBUG_LOG("APP", "Packet enqueued");
                            /// \todo set a flag that TX cycle is running
                            lora_busy = true;
                            break;
                        case LMH_BUSY:
                            DEBUG_LOG("APP", "LoRa transceiver is busy");
                            break;
                        case LMH_ERROR:
                            DEBUG_LOG("APP", "Packet error, too big to send with current DR");
                            break;
                        }
                    }
                }
            }
        }
        // vTaskDelay(5000);
        g_task_event_type = 0;
        // Go back to sleep
        xSemaphoreTake(g_task_sem, 10);
        // Switch off blue LED to show we go to sleep
        digitalWrite(LED_BUILTIN, LOW);
        delay(10);
    }
}

/**
 * @brief Read the battery level as value
 * between 0 and 254. This is used in LoRaWan status requests
 * as the battery level
 * 
 * @return uint8_t Battery level as value between 0 and 254
 */
uint8_t get_lora_batt(void)
{
    return mvToLoRaWanBattVal(readVBAT());
}

/**
 * @brief Initialize LoRa HW and LoRaWan MAC layer
 *
 * @return int8_t result
 *  0 => OK
 * -1 => SX126x HW init failure
 * -2 => LoRaWan MAC initialization failure
 * -3 => Subband selection failure
 */
int8_t init_lorawan(void)
{
    // Initialize LoRa chip.
    if (lora_rak4630_init() != 0)
    {
        DEBUG_LOG("LORA", "Failed to initialize SX1262");
        return -1;
    }

    // Setup the EUIs and Keys
    lmh_setDevEui(g_lorawan_credentials.node_device_eui);
    lmh_setAppEui(g_lorawan_credentials.node_app_eui);
    lmh_setAppKey(g_lorawan_credentials.node_app_key);
    // lmh_setNwkSKey(g_lorawan_settings.node_nws_key);
    // lmh_setAppSKey(g_lorawan_settings.node_apps_key);
    // lmh_setDevAddr(g_lorawan_settings.node_dev_addr);

    // Setup the LoRaWan init structure
    lora_param_init.adr_enable = g_lorawan_settings.adr_enabled;
    lora_param_init.tx_data_rate = g_lorawan_settings.data_rate;
    lora_param_init.enable_public_network = g_lorawan_static_settings.public_network;
    lora_param_init.nb_trials = g_lorawan_settings.join_trials;
    lora_param_init.tx_power = g_lorawan_settings.tx_power;
    lora_param_init.duty_cycle = g_lorawan_static_settings.duty_cycle_enabled;

    // DEBUG_LOG("LORA", "Initialize LoRaWAN for region %s", region_names[g_lorawan_settings.lora_region]);
    // Initialize LoRaWan
    if (lmh_init(&lora_callbacks, lora_param_init, true, (eDeviceClass)g_lorawan_static_settings.lora_class, (LoRaMacRegion_t)g_lorawan_settings.lora_region) != 0)
    {
        DEBUG_LOG("LORA", "Failed to initialize LoRaWAN");
        return -2;
    }

    // For some regions we might need to define the sub band the gateway is listening to
    // This must be called AFTER lmh_init()
    if (!lmh_setSubBandChannels(g_lorawan_settings.subband_channels))
    {
        DEBUG_LOG("LORA", "lmh_setSubBandChannels failed. Wrong sub band requested?");
        return -3;
    }

    DEBUG_LOG("LORA", "Begin join timer");
    // Initialize the app timer
    g_task_lora_tx_wakeup_timer.begin(g_lorawan_settings.send_repeat_time * 1000, lora_tx_wakeup);

    DEBUG_LOG("LORA", "Start Join");
    // Start Join process
    lmh_join();

    g_lorawan_initialized = true;
    return 0;
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

/**************************************************************/
/* LoRaWAN callback functions                                            */
/**************************************************************/
/**
   @brief LoRa function when join has failed
*/
void lpwan_join_fail_handler(void)
{
    DEBUG_LOG("LORA", "OTAA joined failed");
    DEBUG_LOG("LORA", "Check LPWAN credentials and if a gateway is in range");
    // Restart Join procedure
    DEBUG_LOG("LORA", "Restart network join request");
    g_join_result = false;
    // Wake up task to report failed join
    g_task_event_type |= LORA_JOIN_FIN;
    // Notify task about the event
    if (g_task_sem != NULL)
    {
        DEBUG_LOG("LORA", "Join failed, report event");
        xSemaphoreGive(g_task_sem);
    }
}

/**
 * @brief LoRa function for handling HasJoined event.
 */
static void lpwan_joined_handler(void)
{
    digitalWrite(LED_BUILTIN, LOW);

    otaaDevAddr = lmh_getDevAddr();

#if DEBUG > 0
    DEBUG_LOG("LORA", "OTAA joined and got dev address %08lX", otaaDevAddr);

    delay(100); // Just to enable the serial port to send the message
#endif

    g_join_result = true;
    // Wake up task to report succesful join
    g_task_event_type |= LORA_JOIN_FIN;
    // Notify task about the event
    if (g_task_sem != NULL)
    {
        DEBUG_LOG("LORA", "Join success, report event");
        xSemaphoreGive(g_task_sem);
    }
    delay(100); // Just to enable the serial port to send the message

    g_lpwan_has_joined = true;

    // if (g_lorawan_settings.send_repeat_time != 0)
    // {
    //     DEBUG_LOG("LORA", "Start timer");
    //     delay(100); // Just to enable the serial port to send the message
    //     // Now we are connected, start the timer that will wakeup the loop frequently
    //     // g_task_wakeup_timer.start();
    //     DEBUG_LOG("LORA", "Started timer");
    //     delay(100); // Just to enable the serial port to send the message
    // }
}

/**
 * @brief Function for handling LoRaWan received data from Gateway
 *
 * @param app_data  Pointer to rx data
 */
static void lpwan_rx_handler(lmh_app_data_t *app_data)
{
    DEBUG_LOG("LORA", "LoRa Packet received on port %d, size:%d, rssi:%d, snr:%d",
            app_data->port, app_data->buffsize, app_data->rssi, app_data->snr);

    g_last_rssi = app_data->rssi;
    g_last_snr = app_data->snr;
    g_last_fport = app_data->port;

    // Copy the data into loop data buffer
    memcpy(g_rx_lora_data, app_data->buffer, app_data->buffsize);
    g_rx_data_len = app_data->buffsize;
    g_task_event_type |= LORA_DATA;
    // Notify task about the event
    if (g_task_sem != NULL)
    {
        DEBUG_LOG("LORA", "Waking up loop task");
        xSemaphoreGive(g_task_sem);
    }
}

/**
 * @brief Callback for class switch confirmation
 *
 * @param Class The new class
 */
static void lpwan_class_confirm_handler(DeviceClass_t Class)
{
    DEBUG_LOG("LORA", "switch to class %c done", "ABC"[Class]);

    // Wake up task to send initial packet
    g_task_event_type |= STATUS;
    // Notify task about the event
    if (g_task_sem != NULL)
    {
        DEBUG_LOG("LORA", "Waking up loop task");
        xSemaphoreGive(g_task_sem);
    }
    g_lpwan_has_joined = true;
}

/**
 * @brief Called after unconfirmed packet was sent
 *
 */
static void lpwan_unconfirm_tx_finished(void)
{
    DEBUG_LOG("LORA", "Uncomfirmed TX finished");
    g_rx_fin_result = true;
    // Wake up task to send initial packet
    g_task_event_type |= LORA_TX_FIN;
    // Notify task about the event
    if (g_task_sem != NULL)
    {
        DEBUG_LOG("LORA", "Waking up loop task");
        xSemaphoreGive(g_task_sem);
    }
}

/**
 * @brief Called after confirmed packet was sent
 *
 * @param result Result of sending true = ACK received false = No ACK
 */
static void lpwan_confirm_tx_finished(bool result)
{
    DEBUG_LOG("LORA", "Comfirmed TX finished with result %s", result ? "ACK" : "NAK");
    g_rx_fin_result = result;
    // Wake up task to send initial packet
    g_task_event_type |= LORA_TX_FIN;
    // Notify task about the event
    if (g_task_sem != NULL)
    {
        DEBUG_LOG("LORA", "Waking up loop task");
        xSemaphoreGive(g_task_sem);
    }
}

/**
 * @brief Send a LoRaWan package
 *
 * @return result of send request
 */
lmh_error_status send_lora_packet(uint8_t *data, uint8_t size, uint8_t fport)
{
    if (lmh_join_status_get() != LMH_SET)
    {
        // Not joined, try again later
        DEBUG_LOG("LORA", "Did not join network, skip sending frame");
        return LMH_ERROR;
    }

    if (fport != 0)
    {
        m_lora_app_data.port = fport;
    }
    else
    {
        m_lora_app_data.port = g_lorawan_settings.app_port;
    }

    m_lora_app_data.buffsize = size;

    memcpy(m_lora_app_data_buffer, data, size);

    return lmh_send(&m_lora_app_data, g_lorawan_settings.confirmed_msg_enabled);
}