#ifndef RAK1903_LIGHT_H
#define RAK1903_LIGHT_H

#include "main.h"
#include <ClosedCube_OPT3001.h>

extern ClosedCube_OPT3001 opt3001;
extern bool is_rak1903_init;

bool rak1903_init(void);

#endif