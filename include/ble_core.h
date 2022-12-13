#ifndef BLE_CORE_H
#define BLE_CORE_H

// BLE
#include <bluefruit.h>

void init_ble(void);
BLEService init_lorawan_control_characteristics(void);
BLEService init_lorawan_credentials_characteristics(void);
BLEService init_lorawan_settings_characteristics(void);
extern BLEUart ble_uart;
extern BLEBas ble_bas;
extern bool ble_uart_is_connected;

#endif