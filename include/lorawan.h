 // LoRaWAN
#ifndef LORAWAN_H
#define LORAWAN_H

#include <LoRaWan-RAK4630.h>
#include "gnss.h"
#include "temp_hum.h"

int8_t init_lorawan(void);
lmh_error_status send_lora_packet(uint8_t *data, uint8_t size, uint8_t fport = 0);
extern bool g_lpwan_has_joined;
extern bool g_rx_fin_result;
extern bool g_join_result;
extern uint32_t otaaDevAddr;

struct lorawan_payload_s
{
   	uint8_t data[51];
	uint8_t data_length;
	uint8_t fport;
};

extern lorawan_payload_s lorawan_payload;
extern QueueHandle_t xStructQueue;

void lorawan_init();

#endif