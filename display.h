//
// Created by MightyPork on 2017/06/08.
//

#ifndef FIRMWARE_DISPLAY_H
#define FIRMWARE_DISPLAY_H

#include <stdint.h>

//  AAA
// F   B
// F   B
//  GGG
// E   C
// E   C
//  DDD  H

// individual segments
#define SEG_A _BV(0)
#define SEG_B _BV(1)
#define SEG_C _BV(2)
#define SEG_D _BV(3)
#define SEG_E _BV(4)
#define SEG_F _BV(5)
#define SEG_G _BV(6)
#define SEG_H _BV(7)

// composed digits
#define DIGIT_0 (SEG_A|SEG_B|SEG_C|SEG_D|SEG_E|SEG_F)
#define DIGIT_1 (SEG_B|SEG_C)
#define DIGIT_2 (SEG_A|SEG_B|SEG_D|SEG_E|SEG_G)
#define DIGIT_3 (SEG_A|SEG_B|SEG_C|SEG_D|SEG_G)
#define DIGIT_4 (SEG_B|SEG_C|SEG_F|SEG_G)
#define DIGIT_5 (SEG_A|SEG_C|SEG_D|SEG_F|SEG_G)
#define DIGIT_6 (SEG_A|SEG_C|SEG_D|SEG_E|SEG_F|SEG_G)
#define DIGIT_7 (SEG_A|SEG_B|SEG_C|SEG_F)
#define DIGIT_8 (SEG_A|SEG_B|SEG_C|SEG_D|SEG_E|SEG_F|SEG_G)
#define DIGIT_9 (SEG_A|SEG_B|SEG_C|SEG_D|SEG_F|SEG_G)

extern const uint8_t disp_digits[10];
extern volatile uint8_t disp_brightness; // place to globally store display brightness

/**
 * Send raw digits to the display
 * @param dig0 - segments of the left digit
 * @param dig1 - segments of the right digit
 */
void display_show(uint8_t dig0, uint8_t dig1);

/**
 * Show a number 0-99 on the display
 * @param num - number to show
 */
void display_show_number(uint8_t num);

/**
 * Init the PWM timer with automatic adjust from ADC level
 */
void setup_pwm(void);

#endif //FIRMWARE_DISPLAY_H
