//
// Created by MightyPork on 2017/06/08.
// originally written 2016/9/2 for STM32F103 bluepill, adapted
//

#include "game.h"

#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <avr/io.h>
#include <iopins.h>
#include "lib/color.h"
#include "leds.h"
#include "lib/timebase.h"
#include "display.h"
#include "pinout.h"

//region Colors

#define C_DARK rgb24(0,0,0)
#define C_DIMWHITE rgb24(30,30,30)
#define C_OKGREEN rgb24(30,200,0)
#define C_CRIMSON rgb24(255,0,0)

#define C_DIMRED rgb24(100,0,0)
#define C_DIMGREEN rgb24(0,100,0)
#define C_DIMBLUE rgb24(0,0,80)
#define C_DIMYELLOW rgb24(50,45,0)

#define C_BRTRED rgb24(255,0,0)
#define C_BRTGREEN rgb24(0,255,0)
#define C_BRTBLUE rgb24(0,0,255)
#define C_BRTYELLOW rgb24(127,110,0)

// assign to positions

#define C_DIM1 C_DIMRED
#define C_DIM2 C_DIMGREEN
#define C_DIM3 C_DIMBLUE
#define C_DIM4 C_DIMYELLOW

#define C_BRT1 C_BRTRED
#define C_BRT2 C_BRTGREEN
#define C_BRT3 C_BRTBLUE
#define C_BRT4 C_BRTYELLOW

//endregion

enum GameState_enum {
	STATE_NEW_GAME, // new game, waiting for key
	STATE_REPLAY, // showing sequence
	STATE_USER_INPUT, // waiting for user input of repeated sequence
	STATE_SUCCESS_EFFECT, // entered OK, show some fireworks
	STATE_FAIL_EFFECT, // entered wrong, show FAIL animation, then reset.
};

/** Current game state */
enum GameState_enum GameState = STATE_NEW_GAME;

/** Screen colors */
uint32_t screen[4] = {0, 0, 0, 0};
const uint32_t brt[4] = {C_BRT1, C_BRT2, C_BRT3, C_BRT4};
const uint32_t dim[4] = {C_DIM1, C_DIM2, C_DIM3, C_DIM4};
const uint32_t dark[4] = {C_DARK, C_DARK, C_DARK, C_DARK};
const uint32_t dimwhite[4] = {C_DIMWHITE, C_DIMWHITE, C_DIMWHITE, C_DIMWHITE};

#define REPLAY_INTERVAL 400
#define REPLAY_INTERVAL_GAP 75
#define SUC_EFF_TIME 500
#define FAIL_EFF_TIME 1000

/** Sequence of colors to show. Seed is constant thorough a game.
 * rng_state is used by rand_r() for building the sequence. */
uint32_t game_seed;
unsigned long game_rng_state;
uint8_t last_item;
uint8_t repeat_count;

/** Nr of revealed colors in sequence */
uint8_t game_revealed_n;
/** Nr of next color to replay/input */
uint8_t game_replay_n;
/** Nr of succ repeated colors */
uint8_t game_repeat_n;

void enter_state(enum GameState_enum state);

/** Show current screen colors */
void show_screen()
{
	leds_set(screen);
}

/** Prepare rng sequence for replay / test */
void reset_sequence()
{
	game_rng_state = game_seed;
	last_item = 99;
	repeat_count = 0;
}

/** Get next item in the sequence */
uint8_t get_next_item()
{
	uint8_t item;
	while (1) {
		item = (uint8_t) rand_r(&game_rng_state) & 0x03;
		if (item == last_item) {
			repeat_count++;
			if (repeat_count < 2) {
				goto suc;
			}
		} else {
			last_item = item;
			repeat_count = 0;
			goto suc;
		}
	}

suc:
	return item;
}

/** Enter state - callback for delayed state change */
void deferred_enter_state(void *state)
{
	enter_state((enum GameState_enum) state);
}

/** Future task CB in replay seq */
void replay_callback(void *onOff)
{
	bool on = (bool) onOff;

	screen[0] = C_DARK;
	screen[1] = C_DARK;
	screen[2] = C_DARK;
	screen[3] = C_DARK;

	if (on) {
		uint8_t color = get_next_item();
		game_replay_n++;
		screen[color] = brt[color];
		show_screen();
		schedule_task(replay_callback, (void *) 0, REPLAY_INTERVAL, false);
	} else {
		// turning off
		show_screen();

		// Schedule next turning ON
		if (game_replay_n < game_revealed_n) {
			schedule_task(replay_callback, (void *) 1, REPLAY_INTERVAL_GAP, false);
		} else {
			enter_state(STATE_USER_INPUT);
			//schedule_task(deferred_enter_state, (void *) STATE_USER_INPUT, 50, false);
		}
	}
}

/** SUCCESS effect */
void suc_eff_callback(void *onOff)
{
	bool on = (bool) onOff;

	if (on) {
		for (uint8_t i = 0; i < 4; i++) screen[i] = C_OKGREEN;
		schedule_task(suc_eff_callback, 0, SUC_EFF_TIME, false);
	} else {
		for (uint8_t i = 0; i < 4; i++) screen[i] = C_DARK;

		schedule_task(deferred_enter_state, (void *) STATE_REPLAY, 250, false);
	}

	show_screen();
}

/** ERROR effect */
void fail_eff_callback(void *onOff)
{
	bool on = (bool) onOff;

	if (on) {
		for (int i = 0; i < 4; i++) screen[i] = C_CRIMSON;
		schedule_task(fail_eff_callback, 0, FAIL_EFF_TIME, false);
	} else {
		for (int i = 0; i < 4; i++) screen[i] = C_DARK;

		schedule_task(deferred_enter_state, (void *) STATE_NEW_GAME, 250, false);
	}

	show_screen();
}

/**
 * @brief Enter a game state
 * @param state
 */
void enter_state(enum GameState_enum state)
{
	GameState = state;

	switch (state) {
		case STATE_NEW_GAME:
			// new game - idle state before new game is started

			// all dimly lit
			for (int i = 0; i < 4; i++) screen[i] = 0; //C_DIMWHITE
			break;

		case STATE_REPLAY:
			game_replay_n = 0;
			reset_sequence();

			// Start replay
			replay_callback((void *) 1);
			break;

		case STATE_USER_INPUT:
			memcpy(screen, dim, sizeof(screen));

			// Start entering & checking
			game_repeat_n = 0;
			reset_sequence();
			break;

		case STATE_SUCCESS_EFFECT:
			memcpy(screen, dim, sizeof(screen));
			//suc_eff_callback((void *) 1);
			schedule_task(suc_eff_callback, (void *) 1, 250, false);
			break;

		case STATE_FAIL_EFFECT:
			memcpy(screen, dim, sizeof(screen));
			//fail_eff_callback((void *) 1);
			schedule_task(fail_eff_callback, (void *) 1, 250, false);
			break;
	}

	show_screen();
}

/** Prepare new sequence, using time for seed. */
void prepare_sequence()
{
	game_seed = time_ms;
	game_rng_state = game_seed;
	last_item = 99;
	repeat_count = 0;
}

/** Main function, called from MX-generated main.c */
void game_main(void)
{
	display_show(SEG_G, SEG_G); // two dashes...
	enter_state(STATE_NEW_GAME);
	// we'll init the sequence when user first presses a button - the time is used as a seed

	enum GameState_enum last_state = STATE_NEW_GAME;
	uint16_t cnt = 0;
	while (1) {
		if (GameState == last_state) {
			if (GameState == STATE_NEW_GAME) {
				if (cnt == 50) {
					// clear after 5 secs
					display_show(SEG_G, SEG_G);
				}

				if (cnt == 3000) {
					// Shut down after 5 mins
					screen[0] = C_CRIMSON;
					screen[1] = C_CRIMSON;
					screen[2] = C_CRIMSON;
					screen[3] = C_CRIMSON;
					show_screen();
					delay_s(2000);
					pin_down(PIN_PWR_HOLD);
				}
			} else {
				if (cnt > 150) {// 15 secs = stop game.
					// reset state
					enter_state(STATE_NEW_GAME);
					show_screen();
					display_show(SEG_G, SEG_G);
					cnt = 0;
				}
			}
		} else {
			last_state = GameState;
			cnt = 0;
		}

		cnt++;
		delay_ms(100);
	}
}

/**
 * @brief Handle a button press. Callback for debouncer.
 * @param button: button identifier
 * @param press: press state (1 = just pressed, 0 = just released)
 */
void game_button_handler(uint8_t button, bool press)
{
	// convert to 0-3
	button--;

	switch (GameState) {
		case STATE_NEW_GAME:
			if (press) {
				// feedback
				display_show_number(0); // show 0
			}

			if (!press) { // released
				// user wants to start playing
				prepare_sequence();
				game_revealed_n = 1; // start with 1 revealed

				// darken
				//memcpy(screen, dark, sizeof(screen));
				//show_screen();

				// start playback with a delay
				// this makes it obvious the playback is not a feedback to the pressed button
				schedule_task(deferred_enter_state, (void *) STATE_REPLAY, 500, false);
				//enter_state(STATE_REPLAY);
			}
			break;
		case STATE_USER_INPUT:
			// user is entering a color
			memcpy(screen, dim, sizeof(screen));

			if (press) {
				// Button is down
				screen[button] = brt[button];
			} else {
				// Button is released
				// Verify correctness
				uint8_t expected = get_next_item();
				if (expected == button) {
					// good!
					game_repeat_n++;
					if (game_repeat_n == game_revealed_n) {
						// repeated all, good work!
						game_revealed_n++;
						display_show_number(game_revealed_n-1);
						enter_state(STATE_SUCCESS_EFFECT);
					}
				} else {
					enter_state(STATE_FAIL_EFFECT);
				}
			}

			show_screen();
			break;

		default:
			// discard button press, not expecting input now
			break;
	}
}
