//
// Created by MightyPork on 2017/09/16.
//

#include "rng.h"

#include <stdint.h>
#include <stdlib.h>

/** Sequence of colors to show. Seed is constant thorough a game.
 * rng_state is used by rand_r() for building the sequence. */
static uint32_t game_seed;
static unsigned long game_rng_state;
static uint8_t last_item;
static uint8_t repeat_count;

#define NUM_CNT 4
static uint16_t weights[NUM_CNT];

void rng_set_seed(uint32_t seed)
{
	game_seed = seed;
}

void rng_restart(void)
{
	game_rng_state = game_seed;
	repeat_count = 0;
	last_item = 99;
	for (uint8_t i = 0; i < 4; i++) {
		weights[i] = 1;
	}
}

static uint8_t pick_do(void)
{
	uint16_t total = 0;
	for (uint8_t i = 0; i < 4; i++) {
		total += weights[i];
	}
	uint16_t rn = rand_r(&game_rng_state) % total;

	for (uint8_t i = 0; i < 4; i++) {
		if (rn < weights[i]) {
			// got our number
			for (uint8_t j = 0; j < 4; j++) {
				if (i == j) {
					weights[j]/=2;
				}
				else weights[j]+=4;
			}

			return i;
		}
		rn -= weights[i];
	}
	return 0; // this never happens but keeps the compiler happy
}

uint8_t rng_next_item(void)
{
	uint8_t item;
	while (1) {
		item = pick_do();
		if (item == last_item) {
			repeat_count++;
			if (repeat_count < 2) return item;
		} else {
			last_item = item;
			repeat_count = 0;
			return item;
		}
	}
}
