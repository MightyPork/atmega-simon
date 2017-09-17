#include <avr/io.h>          // register definitions
#include <avr/pgmspace.h>    // storing data in program memory
#include <avr/interrupt.h>   // interrupt vectors
#include <util/delay.h>      // delay functions
#include <stdint.h>          // C header for int types like uint8_t
#include <stdbool.h>         // C header for the bool type
#include <stdio.h>
#include <wsrgb.h>

// Include stuff from the library
#include "lib/iopins.h"
#include "lib/usart.h"
#include "lib/spi.h"
#include "lib/adc.h"
#include "lib/debounce.h"
#include "lib/timebase.h"

#include "pinout.h"
#include "display.h"
#include "leds.h"
#include "game.h"

/**
 * Configure pins
 */
void setup_io(void)
{
	as_output(PIN_DISP_CP);
	as_output(PIN_DISP_D);
	pin_up(PIN_DISP_STR);
	as_output(PIN_DISP_STR);
	as_output(PIN_DISP_OE);

	as_input(PIN_KEY_1);
	as_input(PIN_KEY_2);
	as_input(PIN_KEY_3);
	as_input(PIN_KEY_4);

	as_output(PIN_NEOPIXEL);
	pin_up(PIN_NEOPIXEL_PWRN); // turn neopixels OFF - it's a PMOS
	as_output(PIN_NEOPIXEL_PWRN); // configure DDR for output (pull-up becomes hard up)

	as_input(PIN_PWR_KEY);
	as_output(PIN_PWR_HOLD);

	// PIN_LIGHT_SENSE is ADC exclusive, needs no config
}

// --- Debouncer slot allocation constants ---
volatile bool booting = true;

volatile uint16_t time_pwr_pressed = 0;

/** Power button state changed */
void key_cb_power(uint8_t num, bool state)
{
	if (state) {
		time_pwr_pressed = time_ms;
	} else {
		if (booting) {
			// Ignore this one - user still holding BTN after power ON
			usart_puts("Power button released, leaving boot mode.\r\n");
			booting = false;
			return;
		}
	}
}

void setup_debouncer(void)
{
	// Debouncer config
	debo_add(PIN_PWR_KEY, key_cb_power);
	debo_add(PIN_KEY_1, onGameButton);
	debo_add(PIN_KEY_2, onGameButton);
	debo_add(PIN_KEY_3, onGameButton);
	debo_add(PIN_KEY_4, onGameButton);

	// Timer 1 - CTC, to 16000 (1 ms interrupt)
	OCR1A = 16000;
	TIMSK1 |= _BV(OCIE1A);
	TCCR1B |= _BV(WGM12) | _BV(CS10);
}

// SysTick
ISR(TIMER1_COMPA_vect)
{
	// Tick 1 ms
	debo_tick();
	timebase_ms_cb();
	leds_show();
}

// Shut down by just holding the button - better feedback for user
void task_check_shutdown_btn(void *unused) {
	(void)unused;

	if (debo_get_pin(0) // 0 - first
		&& !booting
		&& (time_ms - time_pwr_pressed > 250)) {
		cli();

		ws_no_cli_sei = true;

		usart_puts("Power OFF\r\n");

		uint32_t zeros[4] = {0,0,0,0};
		leds_set(zeros);
		leds_show();

		display_show(0,0);
		_delay_ms(100);
		// Wait for user to release
		while (pin_read(PIN_PWR_KEY));
		_delay_ms(100);

		// shut down
		pin_down(PIN_PWR_HOLD);
		// wait for shutdown
		while(1);
	}
}

/**
 * Main function
 */
void main()
{
	usart_init(BAUD_115200);
	//usart_isr_rx_enable(true); // enable RX interrupt handler

	setup_io();

	// The Arduino bootloader waits ~ 2 seconds after power on listening on UART,
	// which in this case also serves as a debounce delay for the power switch

	pin_up(D13); // the on-board LED (also SPI clk) - indication for the user
	// Stay on - hold the EN pin high
	pin_up(PIN_PWR_HOLD);

	// SPI conf
	// TODO verify the cpha and cpol. those seem to work, but it's a guess
	spi_init_master(SPI_LSB_FIRST, CPOL_1, CPHA_0, SPI_DIV_4);
	adc_init(ADC_PRESC_128);

	// clear
	display_show(0,0);

	setup_pwm();

	setup_debouncer();

	// Turn neopixels power ON - voltage will have stabilized by now
	// and no glitches should occur
	pin_down(PIN_NEOPIXEL_PWRN);
	ws_init();

	add_periodic_task(task_check_shutdown_btn, NULL, 1, 0);

	// globally enable interrupts
	sei();

	usart_puts("Starting game...\r\n");

	game_main();
}
