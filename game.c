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
#include <usart.h>
#include "lib/color.h"
#include "leds.h"
#include "lib/timebase.h"
#include "display.h"
#include "pinout.h"
#include "rng.h"

//region Colors

#define C_DARK rgb24(0,0,0)
#define C_DIMWHITE rgb24(30,30,30)
#define C_OKGREEN rgb24(20,160,0)
#define C_CRIMSON rgb24(220,0,5)

//#define C_DIMRED rgb24(100,0,0)
//#define C_DIMGREEN rgb24(0,100,0)
//#define C_DIMBLUE rgb24(0,0,80)
//#define C_DIMYELLOW rgb24(50,45,0)

#define C_BRTRED rgb24(255,0,0)
#define C_BRTGREEN rgb24(0,255,0)
#define C_BRTBLUE rgb24(0,0,255)
#define C_BRTYELLOW rgb24(127,110,0)

#define C_DIMRED rgb24((int)(255*0.3),0,0)
#define C_DIMGREEN rgb24(0,(int)(255*0.3),0)
#define C_DIMBLUE rgb24(0,0,(int)(255*0.3))
#define C_DIMYELLOW rgb24((int)(127*0.3),(int)(110*0.3),0)

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
	STATE_GAME_STARTING, // new game, initial anim
	STATE_REPLAY, // showing sequence
	STATE_USER_INPUT, // waiting for user input of repeated sequence
	STATE_SUCCESS_EFFECT, // entered OK, show some fireworks
	STATE_FAIL_EFFECT, // entered wrong, show FAIL animation, then reset.
};

/** Current game state */
static enum GameState_enum GameState = STATE_NEW_GAME;

static volatile bool holding_new_game_button = false;

/** Screen colors */
static uint32_t screen[4] = {0, 0, 0, 0};

static const uint32_t brt[4] = {C_BRT1, C_BRT2, C_BRT3, C_BRT4};
static const uint32_t dim[4] = {C_DIM1, C_DIM2, C_DIM3, C_DIM4};
//static const uint32_t dark[4] = {C_DARK, C_DARK, C_DARK, C_DARK};
//static const uint32_t dimwhite[4] = {C_DIMWHITE, C_DIMWHITE, C_DIMWHITE, C_DIMWHITE};

#define REPLAY_INTERVAL 400
#define REPLAY_INTERVAL_GAP 75
#define SUC_EFF_TIME 500

/** Nr of revealed colors in sequence */
static uint8_t game_revealed_n;
/** Nr of next color to replay/input */
static uint8_t game_replay_n;
/** Nr of succ repeated colors */
static uint8_t game_repeat_n;

static void enterGameState(enum GameState_enum state);

void idle_anim_init(void);

void idle_anim(void)
;

/** Show current screen colors */
static void show_screen()
{
	leds_set(screen);
}

/** Enter state - callback for delayed state change */
static void delayedEnterStateCb(void *state)
{
	// clear flag that button was held
	holding_new_game_button = false;

	enterGameState((enum GameState_enum) state);
}

/** starting a game */
static void newGameCountdownCb(void *state)
{
	int steps = (int) state;

	if (steps == 0) {
		display_show(0,0); // clear all
		// start playback with a delay
		// this makes it obvious the playback is not a feedback to the pressed button
		schedule_task(delayedEnterStateCb, (void *) STATE_REPLAY, 500, false);
		return;
	}

	display_show_number((uint8_t) steps);
	steps--;

	schedule_task(newGameCountdownCb, (void *) steps, 500, false);

}

static void fadeInColorsCb(void *stepsToGo)
{
	uint8_t steps = (uint8_t) (int) stepsToGo;
	if (led_brightness_mul < 255) led_brightness_mul += 1;

	steps -= 1;
	if (steps>0) {
		schedule_task(fadeInColorsCb, (void *) (int) steps, 1, false);
	}
}

static void fadeOutColorsCb(void *stepsToGo)
{
	uint8_t steps = (uint8_t) (int) stepsToGo;
	if (led_brightness_mul > 0) led_brightness_mul -= 1;

	steps -= 1;
	if (steps>0) {
		schedule_task(fadeOutColorsCb, (void *) (int) steps, 1, false);
	}
}

/** Future task CB in replay seq */
static void replaySequenceCb(void *onOff)
{
	bool on = (bool) onOff;

	screen[0] = C_DARK;
	screen[1] = C_DARK;
	screen[2] = C_DARK;
	screen[3] = C_DARK;

	if (on) {
		uint8_t color = rng_next_item();
		game_replay_n++;
		screen[color] = brt[color];
		show_screen();
		schedule_task(replaySequenceCb, (void *) 0, REPLAY_INTERVAL, false);
	} else {
		// turning off
		show_screen();

		// Schedule next turning ON
		if (game_replay_n < game_revealed_n) {
			schedule_task(replaySequenceCb, (void *) 1, REPLAY_INTERVAL_GAP, false);
		} else {
			// fade in the input hints
			led_brightness_mul = 0;
			memcpy(screen, dim, sizeof(screen));
			show_screen();
			enterGameState(STATE_USER_INPUT);

			schedule_task(fadeInColorsCb, (void *) 255, 350, false);
		}
	}
}

/** SUCCESS effect */
static void successEffectCb(void *onOff)
{
	bool on = (bool) onOff;

	if (on) {
		display_show_number(game_revealed_n-1);
		for (uint8_t i = 0; i < 4; i++) screen[i] = C_OKGREEN;
		schedule_task(successEffectCb, 0, SUC_EFF_TIME, false);
	} else {
		for (uint8_t i = 0; i < 4; i++) screen[i] = C_DARK;

		schedule_task(delayedEnterStateCb, (void *) STATE_REPLAY, 250, false);
	}

	show_screen();
}

/** ERROR effect */
static void failEffectCb(void *cnt)
{
	int cn = (int)cnt;
	bool on = ((cn)%2)==1;

	if (on) {
		for (int i = 0; i < 4; i++) screen[i] = C_CRIMSON;
	} else {
		for (int i = 0; i < 4; i++) screen[i] = C_DARK;
	}

	show_screen();

	cn--;
	if (cn == 0) {
		schedule_task(delayedEnterStateCb, (void *) STATE_NEW_GAME, 250, false);
	} else {
		schedule_task(failEffectCb, (void *) cn, 250, false);
	}
}

/** Prepare new sequence, using time for seed. */
static void prepareNewSequence()
{
	rng_set_seed(time_ms);
	rng_restart();
}

task_pid_t fadeout_pid;
task_pid_t fadein_pid;

volatile bool first_start = true;
/**
 * @brief Enter a game state
 * @param state
 */
static void enterGameState(enum GameState_enum state)
{
	GameState = state;

	switch (state) {
		case STATE_NEW_GAME:
			usart_puts("State: new game\r\n");
			// new game - idle state before new game is started

			for (int i = 0; i < 4; i++) screen[i] = 0;
			led_brightness_mul = 0; // fading in...

			if (first_start) {
				abort_scheduled_task(fadeout_pid);
				fadein_pid = schedule_task(fadeInColorsCb, (void *) 255, 1, false);
			}
			break;

		case STATE_GAME_STARTING:
			first_start = false;

			usart_puts("game begins\r\n");
			// user wants to start playing
			prepareNewSequence();
			game_revealed_n = 1; // start with 1 revealed

			abort_scheduled_task(fadein_pid);
			fadeout_pid = schedule_task(fadeOutColorsCb, (void *) 255, 1, false);
			schedule_task(newGameCountdownCb, (void *) 5, 500, false);
			break;

		case STATE_REPLAY:
			led_brightness_mul = 255;
			usart_puts("State: replay\r\n");
			game_replay_n = 0;
			rng_restart();
			display_show_number(game_revealed_n-1);

			// Start replay
			replaySequenceCb((void *) 1);
			break;

		case STATE_USER_INPUT:
			usart_puts("State: repeat\r\n");

			// Start entering & checking
			game_repeat_n = 0;
			rng_restart();
			break;

		case STATE_SUCCESS_EFFECT:
			usart_puts("State: succ\r\n");
			memcpy(screen, dim, sizeof(screen));
			//suc_eff_callback((void *) 1);
			schedule_task(successEffectCb, (void *) 1, 250, false);
			break;

		case STATE_FAIL_EFFECT:
			usart_puts("State: fail\r\n");
			memcpy(screen, dim, sizeof(screen));
			//fail_eff_callback((void *) 1);
			schedule_task(failEffectCb, (void *) 5, 250, false);
			break;
	}

	show_screen();
}

volatile uint16_t idle_cnt = 0;



/** game main function */
void game_main(void)
{
	idle_anim_init();

	display_show(0, 0);
	enterGameState(STATE_NEW_GAME);
	// we'll init the sequence when user first presses a button - the time is used as a seed

	enum GameState_enum last_state = STATE_NEW_GAME;
	while (1) {
		if (GameState == last_state) {
			if (GameState == STATE_NEW_GAME) {
				if (!first_start && idle_cnt == 50 && !holding_new_game_button) {
					usart_puts("clear highscore display\r\n");
					display_show(0, 0);

					// start fading
					abort_scheduled_task(fadeout_pid);
					fadein_pid = schedule_task(fadeInColorsCb, (void *) 255, 1, false);
				}

				if (idle_cnt == 3000) {
					usart_puts("automatic shutdown\r\n");
					screen[0] = C_CRIMSON;
					screen[1] = C_CRIMSON;
					screen[2] = C_CRIMSON;
					screen[3] = C_CRIMSON;
					show_screen();
					delay_s(1);
					pin_down(PIN_PWR_HOLD);
					while(1); // wait for shutdown
				}
			} else {
				if (idle_cnt > 300) {
					// reset state
					usart_puts("game reset, user walked away\r\n");
					enterGameState(STATE_NEW_GAME);
					show_screen();
					display_show(0, 0);
					idle_cnt = 49; // fade in rainbow quickly
				}
			}
		} else {
			last_state = GameState;
			idle_cnt = 0;
		}

		idle_cnt++;

		for (int  i = 0; i < 10; i++) {
			delay_ms(10);
			if (GameState == STATE_NEW_GAME) {
				// we can do some idle animation here
				idle_anim();
			}
		}
	}
}

/**
 * @brief Handle a button press. Callback for debouncer.
 * @param button: button identifier
 * @param press: press state (1 = just pressed, 0 = just released)
 */
void onGameButton(uint8_t button, bool press)
{
	// convert to 0-3
	button--;

	switch (GameState) {
		case STATE_NEW_GAME:
			if (press) {
				usart_puts("pressed a new-game button\r\n");
				// feedback
				display_show(SEG_D|SEG_E|SEG_F|SEG_A, SEG_A|SEG_B|SEG_C|SEG_D);
				holding_new_game_button = true;
			}

			if (!press) { // released
				enterGameState(STATE_GAME_STARTING);
			}
			break;

		case STATE_USER_INPUT:
			// Reset idle counter, so it doesn't cut off in the middle of input
			idle_cnt = 0;

			// user is entering a color
			memcpy(screen, dim, sizeof(screen));

			if (press) {
				// Button is down
				screen[button] = brt[button];
			} else {
				// Button is released
				// Verify correctness
				uint8_t expected = rng_next_item();
				if (expected == button) {
					usart_puts("good key!\r\n");
					game_repeat_n++;
					if (game_repeat_n == game_revealed_n) {
						usart_puts("repeated all, good work!\r\n");
						game_revealed_n++;
						enterGameState(STATE_SUCCESS_EFFECT);
					}
				} else {
					usart_puts("oops bad key\r\n");
					enterGameState(STATE_FAIL_EFFECT);
				}
			}

			show_screen();
			break;

		default:
			usart_puts("discard button press, not expecting input now\r\n");
			break;
	}
}

// -------------------------------

const uint8_t anim_step = 1;
const uint8_t anim_max = 75;

xrgb_t color1;
uint8_t step1;
xrgb_t color2;
uint8_t step2;
xrgb_t color3;
uint8_t step3;
xrgb_t color4;
uint8_t step4;

void idle_anim_init(void)
{
	color1 = xrgb(anim_max, 0, 0);
	step1 = 0;

	color2 = xrgb(anim_max, anim_max, 0);
	step2 = 1;

	color3 = xrgb(0, anim_max, 0);
	step3 = 2;

	color4 = xrgb(0, anim_max, anim_max);
	step4 = 3;
}

void idle_anim_oneled(xrgb_t *color, uint8_t *step)
{
	switch (*step) {
		case 0:
			color->g += anim_step;
			if (color->g >= anim_max) *step = *step + 1;
			break;
		case 1:
			color->r -= anim_step;
			if (color->r == 0)  *step = *step + 1;
			break;
		case 2:
			color->b += anim_step;
			if (color->b >= anim_max) *step = *step + 1;
			break;
		case 3:
			color->g -= anim_step;
			if (color->g == 0) *step = *step + 1;
			break;
		case 4:
			color->r += anim_step;
			if (color->r >= anim_max) *step = *step + 1;
			break;
		default:
			color->b -= anim_step;
			if (color->b == 0) *step = 0;
			break;
	}
}


void idle_anim(void)
{
	idle_anim_oneled(&color1, &step1);
	idle_anim_oneled(&color2, &step2);
	idle_anim_oneled(&color3, &step3);
	idle_anim_oneled(&color4, &step4);
	screen[0] = xrgb_rgb24(color1);
	screen[1] = xrgb_rgb24(color2);
	screen[2] = xrgb_rgb24(color3);
	screen[3] = xrgb_rgb24(color4);
	show_screen();
}
