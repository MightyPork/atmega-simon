//
// Created by MightyPork on 2017/06/08.
//

#include <stdint.h>
#include <iopins.h>
#include <spi.h>
#include "pinout.h"

#include "display.h"

const uint8_t disp_digits[10] = {
	DIGIT_0, DIGIT_1, DIGIT_2, DIGIT_3, DIGIT_4, DIGIT_5, DIGIT_6, DIGIT_7, DIGIT_8, DIGIT_9
};

void display_show(uint8_t dig0, uint8_t dig1)
{
	spi_send(dig1);
	spi_send(dig0);
	pin_down(PIN_DISP_STR);
	pin_up(PIN_DISP_STR);
}

void display_show_number(uint8_t num) {
	uint8_t tens = num/10;
	uint8_t ones = num - tens*10;
	uint8_t dig0 = tens ? disp_digits[tens] : 0;
	uint8_t dig1 = disp_digits[ones];
	dig1 |= SEG_H;
	display_show(dig0, dig1);
}
