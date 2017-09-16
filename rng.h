//
// Created by MightyPork on 2017/09/16.
//

#ifndef FIRMWARE_RNG_H
#define FIRMWARE_RNG_H

#include <stdint.h>

void rng_set_seed(uint32_t seed);

void rng_restart(void);

uint8_t rng_next_item(void);

#endif //FIRMWARE_RNG_H
