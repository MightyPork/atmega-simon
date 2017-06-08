//
// Created by MightyPork on 2017/06/08.
//

#ifndef FIRMWARE_GAME_H
#define FIRMWARE_GAME_H

#include <stdint.h>
#include <stdbool.h>

void game_main(void);

void game_button_handler(uint8_t button, bool press);

#endif //FIRMWARE_GAME_H
