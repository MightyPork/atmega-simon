# Simon game with Pro Mini

There is not much documentation for this project because I made exactly one 
copy of the hardware and it'll probably end there. But if anyone tries to 
replicate this, here are some hints:

## Parts

- powered by 4 AA batteries (3 would be enough)
- buck module to 4V, EN pin controlled by microcontroller 
  (and a button through resistor, mcu senses button for soft shutdown etc).
  Those modules can be found on aliexpress. For a replacement, you can just put
  a mosfet there or something, as long as V_sup won't go over 5V.
- arduino pro mini (anything with atmega328p will work)
- 4 neopixels - blinky lights (chained)
- dual 7-seg display unit for score, CC or CA doesn't matter, easy to reverse in code
- 4 buttons or TTP223 touch keys for user input
- 1 photo-transistor and some resistors for ambient light sense
- displays driven by 595 or equivalent (eg 74HC4094)

what to connect to where can be found in `pinout.h`.

I'll send a more detailed schematic on request, if I still find my notes.

## Flashing

The firmware can be flashed using the arduino bootloader (via the makefile),
but that produces a 1s delay on startup when the bootloader waits.
For this reason I've wiped the bootloader and flashed it directly 
with AVR Dragon using ISP programming.
