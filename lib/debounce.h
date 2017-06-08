#ifndef DEBOUNCE_H
#define DEBOUNCE_H

//
//  An implementation of button debouncer.
//
//  ----
//
//  You must provide a config file debo_config.h (next to your main.c)
//
//  A pin is registered like this:
//
//    #define BTN1 12 // pin D12
//    #define BTN2 13
//
//    debo_add(BTN0);  // The function returns number assigned to the pin (0, 1, ...)
//    debo_add_rev(BTN1);  // active low
//    debo_register(&PINB, PB2, 0);  // direct access - register, pin & invert
//
//  Then periodically call the tick function (perhaps in a timer interrupt):
//
//    debo_tick();
//
//  To check if input is active, use
//
//    debo_get_pin(0); // state of input #0 (registered first)
//    debo_get_pin(1); // state of input #1 (registered second)
//

#include <avr/io.h>
#include <stdbool.h>
#include <stdint.h>

#include "calc.h"
#include "iopins.h"

// Update as needed
#define DEBO_CHANNELS 5
#define DEBO_TICKS 50 // ms

/** Debouncer callback; state - state after applying inverse bit */
typedef void(*DebouncerCallback)(uint8_t n, bool state);

/* Internal debouncer entry */
typedef struct
{
	PORT_P reg;    // pointer to IO register
	uint8_t bit;   // bits 6 and 7 of this hold "state" & "invert" flag
	uint8_t count; // number of ticks this was in the new state
	DebouncerCallback callback;
} debo_slot_t;

debo_slot_t debo_slots[DEBO_CHANNELS];

/**
 * Add a pin for debouncing (low level function)
 *
 * @param pin_reg_pointer - PINx pointer
 * @param bit - number of the debounced bit in the register (0-7)
 * @param invert - invert the value when reading (for buttons to GND, for example)
 */
uint8_t debo_register(PORT_P pin_reg_pointer, uint8_t bit, bool invert, DebouncerCallback cbk);

/**
 * Add a pin for debouncing (must be used with constant args).
 * Returns index in debouncer, used for callbacks.
 *
 * Arg: pin number, defined in iopins.h
 */
#define debo_add(pin, callback) debo_register(&_pin(pin), _pn(pin), false, callback)
#define debo_add_rev(pin, callback) debo_register(&_pin(pin), _pn(pin), true, callback)

/**
 * Check debounced pins, should be called periodically.
 * Updates internal states and fires callbacks.
 */
void debo_tick(void);

/**
 * Get a value of debounced pin (after debounce - latched state)
 */
#define debo_get_pin(i) (get_bit(debo_slots[i].bit, 6) ^ get_bit(debo_slots[i].bit, 7))

#endif
