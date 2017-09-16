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

volatile uint8_t led_brightness_mul = 255;
static uint32_t leds[4];

void leds_set(const uint32_t *new_leds)
{
	memcpy(leds, new_leds, 4*sizeof(uint32_t));
}

void leds_show(void)
{
	xrgb_t arr[4];

	float db = (float)disp_brightness / 255.0f;
	if (db < 0.10) db = 0.10;
	float db2 = (float)led_brightness_mul / 255.0f;
	db *= db2;

	for (uint8_t i = 0; i < 4; i++) {
		arr[i].r = (uint8_t) ((float)rgb24_r(leds[i]) * db);
		arr[i].g = (uint8_t) ((float)rgb24_g(leds[i]) * db);
		arr[i].b = (uint8_t) ((float)rgb24_b(leds[i]) * db);
	}

	ws_send_xrgb_array(arr, 4);
	//ws_send_rgb24_array(leds, 4);
}
