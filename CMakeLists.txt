cmake_minimum_required(VERSION 3.7)
project(firmware)

# Fake CMake config for CLion

set(CMAKE_CXX_STANDARD 98)

set(SOURCE_FILES
        lib/calc.h
        lib/iopins.c
        lib/iopins.h
        lib/adc.c
        lib/adc.h
        lib/debounce.c
        lib/debounce.h
        lib/nsdelay.h
        lib/spi.c
        lib/spi.h
        lib/usart.c
        lib/usart.h
        lib/color.c
        lib/color.h
        lib/wsrgb.c
        lib/wsrgb.h
        lib/timebase.c
        lib/timebase.h
        main.c
        pinout.h
        game.c
        game.h
        display.c
        display.h
        leds.c
        leds.h
        game.c
        game.h rng.c rng.h)

include_directories(lib
        /usr/avr/include/)

add_definitions(-D__AVR_ATmega328P__)

add_executable(firmware ${SOURCE_FILES})
