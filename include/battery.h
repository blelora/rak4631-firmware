#ifndef BATTERY_H
#define BATTERY_H

#include "main.h"
#include "ble_core.h"

void battery_init();
uint8_t mvToLoRaWanBattVal(float mvolts);
float readVBAT(void);

#endif