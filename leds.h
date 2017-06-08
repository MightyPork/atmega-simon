//
// Created by MightyPork on 2017/06/08.
//

#ifndef FIRMWARE_LEDS_H
#define FIRMWARE_LEDS_H

#include <stdint.h>

void leds_set(const uint32_t *new_leds);

void leds_show(void);

#endif //FIRMWARE_LEDS_H
