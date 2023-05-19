#ifndef ACCEL_H
#define ACCEL_H

#include "main.h"
#include "modules/rak1904_accel.h"

// struct s_accel {
//     float  temperature  = 0;
//     float  humidity = 0;
// } __attribute__((packed));

// extern s_accel temp_hum;

void accel_init();

#endif