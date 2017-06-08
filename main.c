#include <avr/io.h>          // register definitions
#include <avr/pgmspace.h>    // storing data in program memory
#include <avr/interrupt.h>   // interrupt vectors
#include <util/delay.h>      // delay functions
#include <stdint.h>          // C header for int types like uint8_t
#include <stdbool.h>         // C header for the bool type
#include <stdio.h>

// Include stuff from the library
#include "lib/iopins.h"
#include "lib/usart.h"
#include "lib/spi.h"
#include "lib/adc.h"

#include "pinout.h"

/**
 * Configure pins
 */
void setup_io(void)
{
	as_output(PIN_DISP_CP);
	as_output(PIN_DISP_D);
	as_output(PIN_DISP_STR);

	as_input(PIN_KEY_1);
	as_input(PIN_KEY_2);
	as_input(PIN_KEY_3);
	as_input(PIN_KEY_4);

	as_output(PIN_NEOPIXEL);
	as_output(PIN_DISP_OE);

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
	TIMSK2 |= _BV(TOIE2);
	TCCR2B |= _BV(CS20);

	adc_start_conversion(LIGHT_ADC_CHANNEL);
}

/** ISR that writes the PWM register - to avoid glitches */
ISR(TIMER2_OVF_vect)
{
	// convert in background
	if (adc_ready()) {
		disp_brightness = 255 - adc_read_8bit();
		adc_start_conversion(LIGHT_ADC_CHANNEL);
	}

	OCR2B = disp_brightness;
}

/**
 * Let's gooo
 */
void main()
{
	usart_init(BAUD_115200);
	//usart_isr_rx_enable(true); // enable RX interrupt handler

	adc_init(ADC_PRESC_128);
	setup_io();
	setup_pwm();

	// SPI conf
	// TODO verify the cpha and cpol. those seem to work, but it's a guess
	spi_init_master(SPI_LSB_FIRST, CPOL_1, CPHA_0, SPI_DIV_2);

	// globally enable interrupts
	sei();

	uint8_t cnt = 0;

	char buf[100];
	while (1) {
		pin_down(PIN_DISP_STR);
		spi_send(cnt);
		spi_send(cnt);
		pin_up(PIN_DISP_STR);
		cnt++;

		_delay_ms(100);

		sprintf(buf, "%d\n", disp_brightness);
		usart_puts(buf);
	}
}
