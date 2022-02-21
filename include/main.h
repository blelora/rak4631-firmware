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

// Wake up events
#define NO_EVENT 0
#define STATUS 0b0000000000000001
#define N_STATUS 0b1111111111111110
#define BLE_DATA 0b0000000000000100
#define N_BLE_DATA 0b1111111111111011
#define LORA_DATA 0b0000000000001000
#define N_LORA_DATA 0b1111111111110111
#define LORA_TX_FIN 0b0000000000010000
#define N_LORA_TX_FIN 0b1111111111101111
#define LORA_JOIN_FIN 0b0000000001000000
#define N_LORA_JOIN_FIN 0b1111111110111111

void periodic_wakeup(TimerHandle_t unused);
extern SemaphoreHandle_t g_task_sem;
extern volatile uint16_t g_task_event_type;
extern SoftwareTimer g_task_wakeup_timer;

#define LORAWAN_DATA_MARKER 0x55
struct s_lorawan_settings
{
	uint8_t valid_mark_1 = 0xAA;				// Just a marker for the Flash
	uint8_t valid_mark_2 = LORAWAN_DATA_MARKER; // Just a marker for the Flash

	// Flag if node joins automatically after reboot
	bool auto_join = false;
	// Flag for OTAA or ABP
	bool otaa_enabled = true;
	// OTAA Device EUI MSB
	uint8_t node_device_eui[8] = {0x60, 0x81, 0xF9, 0xF4, 0x0F, 0x98, 0xD4, 0x1D};
	// OTAA Application EUI MSB
	uint8_t node_app_eui[8] = {0x60, 0x81, 0xF9, 0xAD, 0xC8, 0x4C, 0xCF, 0x3E};
	// OTAA Application Key MSB
	uint8_t node_app_key[16] = {0xFB, 0xCF, 0x26, 0xD2, 0x80, 0x3F, 0x96, 0xF1, 0xB0, 0xBA, 0x44, 0xE5, 0x85, 0xF7, 0x73, 0xEB};
	// ABP Network Session Key MSB
	uint8_t node_nws_key[16] = {0x32, 0x3D, 0x15, 0x5A, 0x00, 0x0D, 0xF3, 0x35, 0x30, 0x7A, 0x16, 0xDA, 0x0C, 0x9D, 0xF5, 0x3F};
	// ABP Application Session key MSB
	uint8_t node_apps_key[16] = {0x3F, 0x6A, 0x66, 0x45, 0x9D, 0x5E, 0xDC, 0xA6, 0x3C, 0xBC, 0x46, 0x19, 0xCD, 0x61, 0xA1, 0x1E};
	// ABP Device Address MSB
	uint32_t node_dev_addr = 0x26021FB4;
	// Send repeat time in milliseconds: 2 * 60 * 1000 => 2 minutes
	uint32_t send_repeat_time = 0;
	// Flag for ADR on or off
	bool adr_enabled = false;
	// Flag for public or private network
	bool public_network = true;
	// Flag to enable duty cycle (validity depends on Region)
	bool duty_cycle_enabled = false;
	// Number of join retries
	uint8_t join_trials = 5;
	// TX power 0 .. 15 (validity depends on Region)
	uint8_t tx_power = 0;
	// Data rate 0 .. 15 (validity depends on Region)
	uint8_t data_rate = 3;
	// LoRaWAN class 0: A, 2: C, 1: B is not supported
	uint8_t lora_class = 0;
	// Subband channel selection 1 .. 9
	uint8_t subband_channels = 2;
	// Data port to send data
	uint8_t app_port = 2;
	// Flag to enable confirmed messages
	lmh_confirm confirmed_msg_enabled = LMH_UNCONFIRMED_MSG;
	// Command from BLE to reset device
	bool resetRequest = true;
	// LoRa region
	uint8_t lora_region = LORAMAC_REGION_US915;
};

extern s_lorawan_settings g_lorawan_settings;

void lora_data_handler(void);

#endif // MAIN_H