 // LoRaWAN
#ifndef LORAWAN_H
#define LORAWAN_H

#include <LoRaWan-RAK4630.h>

int8_t init_lorawan(void);
lmh_error_status send_lora_packet(uint8_t *data, uint8_t size, uint8_t fport = 0);
extern bool g_lpwan_has_joined;
extern bool g_rx_fin_result;
extern bool g_join_result;
extern uint32_t otaaDevAddr;

void lorawan_init();

#endif