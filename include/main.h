#ifndef MAIN_H
#define MAIN_H

// Debug output, 0 to disable, 1 to enable
#define DEBUG 1

#if DEBUG > 0
#define DEBUG_LOG(tag, ...)                      \
do                                               \
    {                                            \
        if (tag)                                 \
            PRINTF("[%s] ", tag);                \
        PRINTF(__VA_ARGS__);                     \
        PRINTF("\n\r");                          \
        if (ble_uart_is_connected)               \
            if (tag)                             \
                ble_uart.printf("[%s] ", tag);   \
            ble_uart.printf(__VA_ARGS__);        \
            ble_uart.printf("\n\r");             \
    } while (0)
#else
#define DEBUG_LOG(...)
#endif

#include <Arduino.h>
#include <nrf_nvic.h>

#include "lorawan.h"
#include "ble_core.h"

#include "at_cmd.h"
typedef struct atcmd_s
{
	const char *cmd_name;		   // CMD NAME
	const char *cmd_desc;		   // AT+CMD?
	int (*query_cmd)(void);		   // AT+CMD=?
	int (*exec_cmd)(char *str);	   // AT+CMD=value
	int (*exec_cmd_no_para)(void); // AT+CMD
} atcmd_t;
void at_serial_input(uint8_t cmd);
extern atcmd_t g_user_at_cmd_list[] __attribute__((weak));

// Wake up events
#define NO_EVENT 0
#define STATUS 0b0000000000000001
#define N_STATUS 0b1111111111111110
#define BLE_DATA 0b0000000000000100
#define N_BLE_DATA 0b1111111111111011
#define LORA_DATA 0b0000000000001000
#define N_LORA_DATA 0b1111111111110111
#define BLE_CONFIG 0b0000000000000010
#define N_BLE_CONFIG 0b1111111111111101
#define LORA_TX_FIN 0b0000000000010000
#define N_LORA_TX_FIN 0b1111111111101111
#define LORA_JOIN_FIN 0b0000000001000000
#define N_LORA_JOIN_FIN 0b1111111110111111
#define LORA_TX 0b0000000010000000 
#define N_LORA_TX 0b1111111101111111 

void periodic_wakeup(TimerHandle_t unused);
void lora_tx_wakeup(TimerHandle_t unused);
extern SemaphoreHandle_t g_task_sem;
extern volatile uint16_t g_task_event_type;
extern SoftwareTimer g_task_wakeup_timer;
extern SoftwareTimer g_task_lora_tx_wakeup_timer;

struct s_lorawan_credentials
{
	// OTAA Device EUI MSB
	uint8_t node_device_eui[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	// OTAA Application EUI MSB
	uint8_t node_app_eui[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	// OTAA Application Key MSB
	uint8_t node_app_key[16] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
};

struct s_lorawan_settings
{
	// Send repeat time in milliseconds
	uint32_t send_repeat_time = 10000;
	// Flag for ADR on or off
	bool adr_enabled = false;
	// Number of join retries
	uint8_t join_trials = 5;
	// TX power 0 .. 15 (validity depends on Region)
	uint8_t tx_power = 0;
	// Data rate 0 .. 15 (validity depends on Region)
	uint8_t data_rate = 3;
	// Subband channel selection 1 .. 9
	uint8_t subband_channels = 2;
	// Data port to send data
	uint8_t app_port = 2;
	// Flag to enable confirmed messages
	lmh_confirm confirmed_msg_enabled = LMH_UNCONFIRMED_MSG;
	// LoRa region
	uint8_t lora_region = LORAMAC_REGION_US915;
};
struct s_lorawan_static_settings
{
	// Flag for public or private network
	bool public_network = true;
	// Flag to enable duty cycle (validity depends on Region)
	bool duty_cycle_enabled = false;
	// LoRaWAN class 0: A, 2: C, 1: B is not supported
	uint8_t lora_class = 0;
};

extern s_lorawan_credentials g_lorawan_credentials;
extern s_lorawan_settings g_lorawan_settings;
extern s_lorawan_static_settings g_lorawan_static_settings;
extern uint8_t g_lorawan_credentials_status;
extern uint8_t g_rx_lora_data[];
extern uint8_t g_rx_data_len;
extern uint8_t g_tx_lora_data[];
extern uint8_t g_tx_data_len;
extern bool g_lorawan_initialized;
extern int16_t g_last_rssi;
extern int8_t g_last_snr;

void lora_data_handler(void);

// Flash
void init_flash(void);
bool save_credentials(void);
void log_credentials(void);
void flash_reset(void);

#endif // MAIN_H