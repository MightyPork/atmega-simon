//
// Created by MightyPork on 2017/06/08.
//

#ifndef FIRMWARE_GAME_H
#define FIRMWARE_GAME_H

#include <stdint.h>
#include <stdbool.h>

/**
 * Game entry point
 */
void game_main(void);

/**
 * Button press handler
 *
 * @param button
 * @param press
 */
void onGameButton(uint8_t button, bool press);

#endif //FIRMWARE_GAME_H
