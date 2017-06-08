//
// Created by MightyPork on 2017/06/07.
//

#ifndef FIRMWARE_PINOUT_H
#define FIRMWARE_PINOUT_H

// Pinout defs
#include "lib/iopins.h"

// 74HC4094 chain interface for dual 7seg
#define PIN_DISP_D   D11 // MOSI
#define PIN_DISP_CP  D13 // SCK
#define PIN_DISP_STR D10 // NSS, unused for SPI
#define PIN_DISP_OE  D3  // OE (PWM)

// Touch keys
#define PIN_KEY_1 A0
#define PIN_KEY_2 A1
#define PIN_KEY_3 A2
#define PIN_KEY_4 A3

// We have a conflict with SPI SCK :(
//#define PIN_LED D13

// NeoPixel data
#define PIN_NEOPIXEL D2
#define PIN_NEOPIXEL_PWRN D4 // PMOS gate

// Power
#define PIN_PWR_KEY A4 // Direct input from the power key, used for power-off
#define PIN_PWR_HOLD A5 // Hold the buck enabled. Set 0 for shutdown

// Ambient light sensor (Vdd -> 10k -> * -> photo transistor -> GND)
#define LIGHT_ADC_CHANNEL 6

#endif //FIRMWARE_PINOUT_H
