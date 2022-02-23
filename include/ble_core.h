// BLE
#include <bluefruit.h>

void init_ble(void);
BLEService init_lorawan_credentials_characteristic(void);
extern BLEUart ble_uart;
extern bool ble_uart_is_connected;