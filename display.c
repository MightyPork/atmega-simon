//
// Created by MightyPork on 2017/06/08.
//

#include <stdint.h>
#include <iopins.h>
#include <spi.h>
#include <avr/interrupt.h>
#include <adc.h>
#include "pinout.h"

#include "display.h"

const uint8_t disp_digits[10] = {
	DIGIT_0, DIGIT_1, DIGIT_2, DIGIT_3, DIGIT_4, DIGIT_5, DIGIT_6, DIGIT_7, DIGIT_8, DIGIT_9
};

#define adc_samps_len 16
uint8_t adc_samps[adc_samps_len];
uint8_t adc_samp_i = 0;

#define light_sens_countdown_max 800
uint16_t light_sens_countdown = 0;

volatile uint8_t disp_brightness;

void display_show(uint8_t dig0, uint8_t dig1)
{
	spi_send(0);
	spi_send(0);
	spi_send(dig1);
	spi_send(dig0);
	pin_down(PIN_DISP_STR);
	pin_up(PIN_DISP_STR);
}

void display_show_number(uint8_t num)
{
	uint8_t tens = num / 10;
	uint8_t ones = num - tens * 10;
	uint8_t dig0 = tens ? disp_digits[tens] : 0;
	uint8_t dig1 = disp_digits[ones];
	dig1 |= SEG_H;
	display_show(dig0, dig1);
}

// --- LED display brightness control ---

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
		if (light_sens_countdown++ == light_sens_countdown_max) {
			light_sens_countdown = 0;
			
			uint8_t brt = 255 - adc_read_8bit(); // inverse
			if (brt < 10) brt = 10;
			adc_start_conversion(LIGHT_ADC_CHANNEL);
			
			// averaging
			adc_samps[adc_samp_i++] = brt;
			if (adc_samp_i == adc_samps_len) adc_samp_i = 0;
			uint16_t sum = 0;
			for (uint8_t i = 0; i < adc_samps_len; i++) {
				sum += adc_samps[i];
			}
			sum /= adc_samps_len;
			disp_brightness = sum;
		}
	}

	OCR2B = disp_brightness;
}
