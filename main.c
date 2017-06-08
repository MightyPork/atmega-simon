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

#include "pinout.h"
#include "display.h"

/**
 * Configure pins
 */
void setup_io(void)
{
	as_output(PIN_DISP_CP);
	as_output(PIN_DISP_D);
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

// --- LED display brightness control ---
volatile uint8_t disp_brightness;
#define LIGHT_ADC_CHANNEL 6

/**
 * PWM for LED display dimming
 */
void setup_pwm(void)
{
	OCR2B = disp_brightness = 0xFF;
	TCCR2A |= _BV(WGM20) | _BV(WGM21) | _BV(COM2B1);
	TIMSK2 |= _BV(TOIE2); // enable ISR
	TCCR2B |= _BV(CS20);

	adc_start_conversion(LIGHT_ADC_CHANNEL);
}

/** ISR that writes the PWM register - to avoid glitches */
ISR(TIMER2_OVF_vect)
{
	// convert in background
	if (adc_ready()) {
		disp_brightness = 255 - adc_read_8bit(); // inverse
		adc_start_conversion(LIGHT_ADC_CHANNEL);
	}

	OCR2B = disp_brightness;
}


// --- Debouncer slot allocation constants ---
volatile bool booting = true;
volatile uint16_t time_ms = 0;

// (normally those would be retvals from debo_add())
#define DB_KEY_POWER 0
#define DB_KEY_1     1
#define DB_KEY_2     2
#define DB_KEY_3     3
#define DB_KEY_4     4

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

/** Button state changed */
void key_cb_button(uint8_t num, bool state)
{
	// TODO
	// num - 1,2,3,4
	usart_puts("BTN ");
	usart_tx('0'+num);
	usart_tx(' ');
	usart_tx('0'+state);
	usart_puts("\r\n");
}

void setup_debouncer(void)
{
	// Debouncer config
	debo_add(PIN_PWR_KEY, key_cb_power);
	debo_add(PIN_KEY_1, key_cb_button);
	debo_add(PIN_KEY_2, key_cb_button);
	debo_add(PIN_KEY_3, key_cb_button);
	debo_add(PIN_KEY_4, key_cb_button);

	// Timer 1 - CTC, to 16000 (1 ms interrupt)
	OCR1A = 16000;
	TIMSK1 |= _BV(OCIE1A);
	TCCR1B |= _BV(WGM12) | _BV(CS10);
}

ISR(TIMER1_COMPA_vect)
{
	// Tick 1 ms
	debo_tick();
	time_ms++;

	// Shut down by just holding the button - better feedback for user
	if (debo_get_pin(DB_KEY_POWER)
		&& !booting
		&& (time_ms - time_pwr_pressed > 1000)) {
		usart_puts("Power OFF\r\n");
		// shut down
		pin_down(PIN_PWR_HOLD);
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
	spi_init_master(SPI_LSB_FIRST, CPOL_1, CPHA_0, SPI_DIV_2);
	adc_init(ADC_PRESC_128);
	setup_pwm();

	setup_debouncer();

	// Turn neopixels power ON - voltage will have stabilized by now
	// and no glitches should occur
	pin_down(PIN_NEOPIXEL_PWRN);
	ws_init();

	// globally enable interrupts
	sei();

	uint32_t pixels[4] = {0xFF0000, 0xFFFF00, 0x00FF00, 0x0000FF};

	uint8_t cnt = 0;

	char buf[100];
	while (1) {
		display_show_number(cnt);
		cnt++;
		cnt = cnt % 100;

		ws_send_rgb24_array(pixels, 4);
		_delay_ms(300);

		sprintf(buf, "BRT = %d\r\n", disp_brightness);
		usart_puts(buf);
	}
}
