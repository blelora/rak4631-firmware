#ifndef RAK12500_GNSS_H
#define RAK12500_GNSS_H

#include "main.h"
#include <SparkFun_u-blox_GNSS_Arduino_Library.h>

extern SFE_UBLOX_GNSS g_myGNSS;
extern bool is_rak12500_init;

bool rak12500_init(void);

#endif