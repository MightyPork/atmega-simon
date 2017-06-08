//
// Created by MightyPork on 2017/06/08.
//

#include <stdint.h>
#include "lib/color.h"
#include "lib/wsrgb.h"
#include "display.h"
#include <math.h>

#ifndef FIRMWARE_LEDS_H
#define FIRMWARE_LEDS_H

uint32_t leds[4];

void leds_set(uint32_t L1, uint32_t L2, uint32_t L3, uint32_t L4)
{
	leds[0] = L1;
	leds[1] = L2;
	leds[2] = L3;
	leds[3] = L4;
}

void leds_show(void)
{
	xrgb_t arr[4];
	for (uint8_t i = 0; i < 4; i++) {
		float db = (float)disp_brightness / 255.0f;
		arr[i].r = (uint8_t) ((float)rgb24_r(leds[i]) * db);
		arr[i].g = (uint8_t) ((float)rgb24_g(leds[i]) * db);
		arr[i].b = (uint8_t) ((float)rgb24_b(leds[i]) * db);
	}

	ws_send_xrgb_array(arr, 4);
	//ws_send_rgb24_array(leds, 4);
}

#endif //FIRMWARE_LEDS_H
