//
// Created by MightyPork on 2017/06/08.
//

#include <stdint.h>
#include "lib/color.h"
#include "lib/wsrgb.h"
#include "display.h"
#include <math.h>
#include <string.h>

#include "leds.h"

static uint32_t leds[4];

void leds_set(const uint32_t *new_leds)
{
	memcpy(leds, new_leds, 4*sizeof(uint32_t));
}

void leds_show(void)
{
	xrgb_t arr[4];
	for (uint8_t i = 0; i < 4; i++) {
		float db = (float)disp_brightness / 255.0f;
		if (db < 0.15) db = 0.15;
		arr[i].r = (uint8_t) ((float)rgb24_r(leds[i]) * db);
		arr[i].g = (uint8_t) ((float)rgb24_g(leds[i]) * db);
		arr[i].b = (uint8_t) ((float)rgb24_b(leds[i]) * db);
	}

	ws_send_xrgb_array(arr, 4);
	//ws_send_rgb24_array(leds, 4);
}
