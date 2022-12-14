#ifndef TEMP_HUM_H
#define TEMP_HUM_H

#include "main.h"
#include "modules/rak1901_temp_hum.h"

struct s_temp_hum {
    float  temperature  = 0;
    float  humidity = 0;
} __attribute__((packed));

extern s_temp_hum temp_hum;

void temp_hum_init();

#endif