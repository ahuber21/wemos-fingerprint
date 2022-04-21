#ifndef STATUSLEDS_H
#define STATUSLEDS_H

#include <Arduino.h>

#define LED_GREEN D6 // on when ready for a command
#define LED_BLUE D7  // on when handling a command
#define LED_RED D8   // on during normal operation, blinking during power-up

// set the pin mode and initally set all leds to low
void setup_leds();

// repeatedly turn an led on and off
void blink_led(uint8_t pin, uint8_t times = 5, uint16_t delay_ms = 150);

#endif // STATUSLEDS_H