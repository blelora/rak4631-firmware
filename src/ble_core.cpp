#include "main.h"

// BLE DFU service
BLEDfu ble_dfu;
// BLE Device Information Service
BLEDis ble_dis;
// BLE UART Service
BLEUart ble_uart;

BLEService        lorawan_credential_service = BLEService(0xF0B0);
BLECharacteristic lorawan_credential_char = BLECharacteristic(0xF0B1);

// Settings callback
void lorawan_credentials_rx_callback(uint16_t conn_hdl, BLECharacteristic *chr, uint8_t *data, uint16_t len);

// Forward declarations for functions
void connect_callback(uint16_t conn_handle);
void disconnect_callback(uint16_t conn_handle, uint8_t reason);
// BLE UART RX callback
void bleuart_rx_callback(uint16_t conn_handle);

bool ble_uart_is_connected = false;

// Initialize BLE and start advertising
void init_ble(void)
{
    // Config the peripheral connection with maximum bandwidth
    // more SRAM required by SoftDevice
    // Note: All config***() function must be called before begin()
    Bluefruit.configPrphBandwidth(BANDWIDTH_MAX);
    Bluefruit.configPrphConn(92, BLE_GAP_EVENT_LENGTH_MIN, 16, 16);

    Bluefruit.begin(1, 0);
    // Set max power. Accepted values are: -40, -30, -20, -16, -12, -8, -4, 0, 4
    Bluefruit.setTxPower(4);
    // Set the BLE device name
    Bluefruit.setName("4631");

    // Set DIS Info
    ble_dis.setManufacturer("RAKwireless");
	ble_dis.setModel("RAK4631");

    Bluefruit.Periph.setConnectCallback(connect_callback);
    Bluefruit.Periph.setDisconnectCallback(disconnect_callback);

    // Start DFU Service 
    ble_dfu.begin();
    // Start DIS Service
    ble_dis.begin();
    // Start the UART service
	ble_uart.begin();
    ble_uart.setRxCallback(bleuart_rx_callback);

    // Initialize the LoRa setting service
	BLEService lorawan_creds_s = init_lorawan_credentials_characteristic();
    
    // Set up and start advertising
    // Advertising packet
    Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
    Bluefruit.Advertising.addTxPower();
    Bluefruit.Advertising.addService(ble_dfu);
    Bluefruit.Advertising.addName();

    // Start Advertising
    Bluefruit.Advertising.restartOnDisconnect(true);
    Bluefruit.Advertising.setInterval(32, 244); // in unit of 0.625 ms
    Bluefruit.Advertising.setFastTimeout(30);   // number of seconds in fast mode
    Bluefruit.Advertising.start(0);             // 0 = Don't stop advertising after n seconds
}

// Callback when client connects
void connect_callback(uint16_t conn_handle)
{
    (void)conn_handle;
    ble_uart_is_connected = true;
    DEBUG_LOG("BLE", "BLE Connected");
}

// Callback invoked when a connection is dropped
void disconnect_callback(uint16_t conn_handle, uint8_t reason)
{
    (void)conn_handle;
    (void)reason;
    ble_uart_is_connected = false;
    DEBUG_LOG("BLE", "BLE Disconnected");
}

// BLE UART RX Callback
void bleuart_rx_callback(uint16_t conn_handle)
{
	(void)conn_handle;

	g_task_event_type |= BLE_DATA;
	xSemaphoreGiveFromISR(g_task_sem, pdFALSE);
}

/**
 * @brief Initialize the settings characteristic
 * 
 */
BLEService init_lorawan_credentials_characteristic(void)
{
	// Initialize the LoRa setting service
	lorawan_credential_service.begin();
	lorawan_credential_char.setProperties(CHR_PROPS_NOTIFY | CHR_PROPS_READ | CHR_PROPS_WRITE);
	lorawan_credential_char.setPermission(SECMODE_OPEN, SECMODE_OPEN);
	lorawan_credential_char.setFixedLen(sizeof(s_lorawan_credentials) + 1);
	lorawan_credential_char.setWriteCallback(lorawan_credentials_rx_callback);

	lorawan_credential_char.begin();

	lorawan_credential_char.write((void *)&g_lorawan_credentials, sizeof(s_lorawan_credentials));

	return lorawan_credential_service;
}

/**
 * Callback if data has been sent from the connected client
 * @param conn_hdl
 * 		The connection handle
 * @param chr
 *      The called characteristic
 * @param data
 *      Pointer to received data
 * @param len
 *      Length of the received data
 */
void lorawan_credentials_rx_callback(uint16_t conn_hdl, BLECharacteristic *chr, uint8_t *data, uint16_t len)
{
	DEBUG_LOG("SETT", "Credentials received");

	delay(1000);

	// Check the characteristic
	if (chr->uuid == lorawan_credential_char.uuid)
	{
		if (len != sizeof(s_lorawan_credentials))
		{
			DEBUG_LOG("SETT", "Received credentials have wrong size %d", len);
			return;
		}

		s_lorawan_credentials *rcvdCredentials = (s_lorawan_credentials *)data;
		if ((rcvdCredentials->valid_mark_1 != 0xAA) || (rcvdCredentials->valid_mark_2 != LORAWAN_CREDENTIALS_MARKER))
		{
			DEBUG_LOG("SETT", "Received credentials data do not have required markers");
			return;
		}

		// Save new LoRa settings
		memcpy((void *)&g_lorawan_credentials, data, sizeof(s_lorawan_credentials));

		// Save new credentials
		save_credentials();

		// Update credentials
		lorawan_credential_char.write((void *)&g_lorawan_credentials, sizeof(s_lorawan_credentials));

		// Inform connected device about new credentials
		lorawan_credential_char.notify((void *)&g_lorawan_credentials, sizeof(s_lorawan_credentials));

		// Notify task about the event
		if (g_task_sem != NULL)
		{
			g_task_event_type |= BLE_CONFIG;
			DEBUG_LOG("SETT", "Waking up loop task");
			xSemaphoreGive(g_task_sem);
		}
	}
}