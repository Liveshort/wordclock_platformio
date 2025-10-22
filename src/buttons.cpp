#include <Arduino.h>

#include "main.h"
#include "buttons.h"

#define DEBOUNCE_DELAY 200 // milliseconds

void IRAM_ATTR button22_isr() {
    unsigned long interrupt_time = millis();
    if (interrupt_time - TIMERS[INTERRUPT_DEBOUNCE] > DEBOUNCE_DELAY) {
        BUTTONS_PRESSED[BUTTON_22] = true;
        TIMERS[INTERRUPT_DEBOUNCE] = interrupt_time;
    }
}


void IRAM_ATTR button23_isr() {
    unsigned long interrupt_time = millis();
    if (interrupt_time - TIMERS[INTERRUPT_DEBOUNCE] > DEBOUNCE_DELAY) {
        BUTTONS_PRESSED[BUTTON_23] = true;
        TIMERS[INTERRUPT_DEBOUNCE] = interrupt_time;
    }
}


void initialize_buttons() {
    pinMode(22, INPUT_PULLUP);
    pinMode(23, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(22), button22_isr, FALLING);
    attachInterrupt(digitalPinToInterrupt(23), button23_isr, FALLING);
}