/** Copyright 2015 Josh Pieper, jjp@pobox.com.  All rights reserved. */

#ifndef MIC_H
#define MIC_H

#include <inttypes.h>

void mic_init(void);
void mic_poll(void);
void mic_timer_update(void);

int32_t mic_last_power(void);
int32_t mic_max_power(void);
int16_t mic_last_value(void);

#endif
