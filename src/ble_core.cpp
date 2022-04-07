#include "main.h"

// BLE DFU service
BLEDfu ble_dfu;
// BLE Device Information Service
BLEDis ble_dis;
// BLE UART Service
BLEUart ble_uart;

BLEService        lorawan_control_service = BLEService("AAA00000-0000-0000-0000-123456789ABC");
BLECharacteristic lorawan_control_char = BLECharacteristic("AAA10000-0000-0000-0000-123456789ABC");
                                
BLEService        lorawan_credential_service = BLEService("BBB00000-0000-0000-0000-123456789ABC");
BLECharacteristic lorawan_credential_data_char = BLECharacteristic("BBB10000-0000-0000-0000-123456789ABC");
BLECharacteristic lorawan_credential_status_char = BLECharacteristic("BBB20000-0000-0000-0000-123456789ABC");

BLEService        lorawan_settings_service = BLEService("CCC00000-0000-0000-0000-123456789ABC");
BLECharacteristic lorawan_settings_data_char = BLECharacteristic("CCC10000-0000-0000-0000-123456789ABC");
BLECharacteristic lorawan_settings_status_char = BLECharacteristic("CCC20000-0000-0000-0000-123456789ABC");


// Settings callback
void lorawan_control_rx_callback(uint16_t conn_hdl, BLECharacteristic *chr, uint8_t *data, uint16_t len);
void lorawan_credentials_rx_callback(uint16_t conn_hdl, BLECharacteristic *chr, uint8_t *data, uint16_t len);
void lorawan_settings_rx_callback(uint16_t conn_hdl, BLECharacteristic *chr, uint8_t *data, uint16_t len);

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

	// Initialize the LoRa control service
	BLEService lorawan_control_s = init_lorawan_control_characteristics();

    // Initialize the LoRa control service
	BLEService lorawan_creds_s = init_lorawan_credentials_characteristics();

	// Initialize the LoRa control service
	BLEService lorawan_settings_s = init_lorawan_settings_characteristics();
    
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
BLEService init_lorawan_control_characteristics(void)
{
	// Initialize the credential control char
	lorawan_control_service.begin();
	lorawan_control_char.setProperties(CHR_PROPS_NOTIFY | CHR_PROPS_READ | CHR_PROPS_WRITE);
	lorawan_control_char.setPermission(SECMODE_OPEN, SECMODE_OPEN);
	lorawan_control_char.setFixedLen(1);
	lorawan_control_char.setWriteCallback(lorawan_control_rx_callback);

	lorawan_control_char.begin();

	lorawan_control_char.write8(0);


	return lorawan_control_service;
}

/**
 * @brief Initialize the credential characteristics
 * 
 */
BLEService init_lorawan_credentials_characteristics(void)
{
	// Initialize the credential data char
	lorawan_credential_service.begin();
	lorawan_credential_data_char.setProperties(CHR_PROPS_NOTIFY | CHR_PROPS_READ | CHR_PROPS_WRITE);
	lorawan_credential_data_char.setPermission(SECMODE_OPEN, SECMODE_OPEN);
	lorawan_credential_data_char.setFixedLen(sizeof(s_lorawan_credentials) + 1);
	lorawan_credential_data_char.setWriteCallback(lorawan_credentials_rx_callback);

	lorawan_credential_data_char.begin();

	lorawan_credential_data_char.write((void *)&g_lorawan_credentials, sizeof(s_lorawan_credentials));

    // Initialize the credential status char
	lorawan_credential_status_char.setProperties(CHR_PROPS_NOTIFY | CHR_PROPS_READ);
	lorawan_credential_status_char.setPermission(SECMODE_OPEN, SECMODE_OPEN);
	lorawan_credential_status_char.setFixedLen(1);

	lorawan_credential_status_char.begin();

	lorawan_credential_status_char.write8(0);

	return lorawan_credential_service;
}

/**
 * @brief Initialize the settings characteristics
 * 
 */
BLEService init_lorawan_settings_characteristics(void)
{
	// Initialize the settings data char
	lorawan_settings_service.begin();
	lorawan_settings_data_char.setProperties(CHR_PROPS_NOTIFY | CHR_PROPS_READ | CHR_PROPS_WRITE);
	lorawan_settings_data_char.setPermission(SECMODE_OPEN, SECMODE_OPEN);
	lorawan_settings_data_char.setFixedLen(sizeof(s_lorawan_settings) + 1);
	lorawan_settings_data_char.setWriteCallback(lorawan_settings_rx_callback);

	lorawan_settings_data_char.begin();

	lorawan_settings_data_char.write((void *)&g_lorawan_settings, sizeof(s_lorawan_settings));

    // Initialize the settings status char
	lorawan_settings_status_char.setProperties(CHR_PROPS_NOTIFY | CHR_PROPS_READ);
	lorawan_settings_status_char.setPermission(SECMODE_OPEN, SECMODE_OPEN);
	lorawan_settings_status_char.setFixedLen(1);

	lorawan_settings_status_char.begin();

	lorawan_settings_status_char.write8(0);

	return lorawan_settings_service;
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
void lorawan_control_rx_callback(uint16_t conn_hdl, BLECharacteristic *chr, uint8_t *data, uint16_t len)
{
	DEBUG_LOG("SETT", "LoRaWAN Control Write received");

	delay(1000);

	// Check the characteristic
	if (chr->uuid == lorawan_control_char.uuid)
	{
		uint8_t *rcvdControl = data;
		if (len != 1)
		{
			DEBUG_LOG("SETT", "Received LoRaWAN Control Write has wrong size %d", len);
		}
		else
		{
			DEBUG_LOG("SETT", "Received LoRaWAN Control Write with data %d and correct size %d", *rcvdControl, len);

			if(*rcvdControl == 1)
			{
				switch(init_lorawan()) {
					case -1 :
						DEBUG_LOG("LORA", "Failed to initialize SX1262");
						lorawan_control_char.notify8(2);
						lorawan_control_char.write8(2);
						break;
					case -2 :
						DEBUG_LOG("LORA", "Failed to initialize LoRaWAN");
						lorawan_control_char.notify8(3);
						lorawan_control_char.write8(3);
						break;
					case -3 :
						DEBUG_LOG("LORA", "lmh_setSubBandChannels failed. Wrong sub band requested?");
						lorawan_control_char.notify8(4);
						lorawan_control_char.write8(4);
						break;
					default :
						g_task_lora_tx_wakeup_timer.start();

						// init lorawan successful
						lorawan_control_char.notify8(1);
						lorawan_control_char.write8(1);
				}
			}
			else if(*rcvdControl == 0)
			{
				g_task_lora_tx_wakeup_timer.stop();

				// lorawan tx stopped
				lorawan_control_char.notify8(0);
				lorawan_control_char.write8(0);
			}
		}
	}
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
	DEBUG_LOG("SETT", "LoRaWAN Credential Write received");

	delay(1000);

	// Check the characteristic
	if (chr->uuid == lorawan_credential_data_char.uuid)
	{
		if (len != sizeof(s_lorawan_credentials))
		{
			DEBUG_LOG("SETT", "Received credentials have wrong size %d", len);

			// Inform connected device about incorrectnew credentials
			lorawan_credential_status_char.notify8(2);

			// Inform connected device about incorrect new credentials
			lorawan_credential_status_char.write8(0);
		}
		else
		{

			DEBUG_LOG("SETT", "Received credentials with correct size %d, len");

			// Save new LoRa settings
			memcpy((void *)&g_lorawan_credentials, data, sizeof(s_lorawan_credentials));

			// Save new credentials
			save_credentials();

			// Update credentials
			lorawan_credential_data_char.write((void *)&g_lorawan_credentials, sizeof(s_lorawan_credentials));

			// Inform connected device about new credentials
			lorawan_credential_data_char.notify((void *)&g_lorawan_credentials, sizeof(s_lorawan_credentials));

			// Inform connected device about valid new credentials
			lorawan_credential_status_char.notify8(1);

			// Inform connected device about valid new credentials
			lorawan_credential_status_char.write8(0);
		}
		// Notify task about the event
		if (g_task_sem != NULL)
		{
			g_task_event_type |= BLE_CONFIG;
			DEBUG_LOG("SETT", "Waking up loop task");
			xSemaphoreGive(g_task_sem);
		}
	}
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
void lorawan_settings_rx_callback(uint16_t conn_hdl, BLECharacteristic *chr, uint8_t *data, uint16_t len)
{
	DEBUG_LOG("SETT", "LoRaWAN Settings Write received");

	delay(1000);

	// Check the characteristic
	if (chr->uuid == lorawan_settings_data_char.uuid)
	{
		if (len != sizeof(s_lorawan_settings))
		{
			DEBUG_LOG("SETT", "Received settings have wrong size %d", len);

			// Inform connected device about incorrectnew credentials
			lorawan_settings_status_char.notify8(2);

			// Inform connected device about incorrect new credentials
			lorawan_settings_status_char.write8(0);
		}
		else
		{

			DEBUG_LOG("SETT", "Received settings with correct size %d, len");

			// Save new LoRa settings
			memcpy((void *)&g_lorawan_settings, data, sizeof(s_lorawan_settings));

			// Save new credentials
			save_settings();

			// Update credentials
			lorawan_settings_data_char.write((void *)&g_lorawan_settings, sizeof(s_lorawan_settings));

			// Inform connected device about new credentials
			lorawan_settings_data_char.notify((void *)&g_lorawan_settings, sizeof(s_lorawan_settings));

			// Inform connected device about valid new credentials
			lorawan_settings_status_char.notify8(1);

			// Inform connected device about valid new credentials
			lorawan_settings_status_char.write8(0);
		}
		// Notify task about the event
		if (g_task_sem != NULL)
		{
			g_task_event_type |= BLE_CONFIG;
			DEBUG_LOG("SETT", "Waking up loop task");
			xSemaphoreGive(g_task_sem);
		}
	}
}