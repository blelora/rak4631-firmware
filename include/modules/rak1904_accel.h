#ifndef RAK1904_ACCEL_H
#define RAK1904_ACCEL_H

#include "main.h"
#include <Adafruit_LIS3DH.h>

extern Adafruit_LIS3DH lis;
extern bool is_rak1904_init;

bool rak1904_init(void);

#endif