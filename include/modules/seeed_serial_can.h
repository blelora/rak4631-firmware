#ifndef SEEED_SERIAL_CAN_H
#define SEEED_SERIAL_CAN_H

#include "main.h"
#include <Serial_CAN_Module.h>

extern Serial_CAN can;
extern bool is_serial_can_init;

bool serial_can_init(void);
bool getVIN(uint8_t *buf);
bool getSpeed(uint8_t *s);
bool getRPM(uint16_t *r);
#endif