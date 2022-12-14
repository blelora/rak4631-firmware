#ifndef RAK1901_TEMP_HUM_H
#define RAK1901_TEMP_HUM_H

#include "main.h"
#include "SparkFun_SHTC3.h" 

extern SHTC3 mySHTC3;
extern bool is_rak1901_init;

bool rak1901_init(void);

#endif