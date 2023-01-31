#ifndef OBD_H
#define OBD_H

#include "main.h"
#include "modules/seeed_serial_can.h"

struct s_obd_data {
    uint8_t vehicle_speed = 0;
    uint16_t vehicle_rpm = 0;
    uint8_t vin[17];
} __attribute__((packed));

void obd_init();

#endif