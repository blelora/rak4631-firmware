#include "main.h"

// BLE DFU service
BLEDfu ble_dfu;
// BLE Device Information Service
BLEDis ble_dis;
// BLE UART Service
BLEUart ble_uart;

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