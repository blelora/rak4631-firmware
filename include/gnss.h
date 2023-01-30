#ifndef GNSS_H
#define GNSS_H

#include "main.h"
#include "modules/rak12500_gnss.h"

struct s_gnss_location {
    int32_t  latitude  = 0;
    int32_t  longitude = 0;
    uint8_t  accuracy = 0;
} __attribute__((packed));

extern s_gnss_location gnss_location;

void gnss_init();

#endif