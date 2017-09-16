//
// Created by MightyPork on 2017/06/08.
//

#ifndef FIRMWARE_LEDS_H
#define FIRMWARE_LEDS_H

#include <stdint.h>

/**
 * Set led colors
 *
 * @param new_leds - array of 4 ints for the 4 leds
 */
void leds_set(const uint32_t *new_leds);

/**
 * Send the set colors to the neopixels
 */
void leds_show(void);

#endif //FIRMWARE_LEDS_H
