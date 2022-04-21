#include "StatusLEDs.h"
#include "Arduino.h"

void setup_leds()
{
    pinMode(LED_GREEN, OUTPUT);
    pinMode(LED_BLUE, OUTPUT);
    pinMode(LED_RED, OUTPUT);

    digitalWrite(LED_GREEN, LOW);
    digitalWrite(LED_BLUE, LOW);
    digitalWrite(LED_RED, LOW);
}

void blink_led(uint8_t pin, uint8_t times, uint16_t delay_ms)
{
    bool orig_state = digitalRead(pin);

    for (uint8_t i = 0; i < 5; ++i)
    {
        digitalWrite(pin, HIGH);
        delay(delay_ms);
        digitalWrite(pin, LOW);
        delay(delay_ms);
    }

    digitalWrite(pin, orig_state);
}