#ifndef BATTERY_H
#define BATTERY_H

#include "main.h"
#include "ble_core.h"
#include "lorawan.h"

struct s_battery_status {
    uint8_t  capacity = 0;
    uint8_t charging = 0;
} __attribute__((packed));

void battery_init();
uint8_t mvToLoRaWanBattVal(float mvolts);
float readVBAT(void);
uint8_t mvToPercent(float mvolts);

#endif