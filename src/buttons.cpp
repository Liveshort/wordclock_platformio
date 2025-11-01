#include <Arduino.h>

#include "buttons.h"
#include "main.h"

#define DEBOUNCE_DELAY 200  // milliseconds

void IRAM_ATTR button_dimmer_isr() {
    unsigned long interrupt_time = millis();
    if (interrupt_time - TIMERS[INTERRUPT_DEBOUNCE] > DEBOUNCE_DELAY) {
        BUTTONS_PRESSED[BUTTON_DIMMER] = true;
        TIMERS[INTERRUPT_DEBOUNCE] = interrupt_time;
    }
}

void IRAM_ATTR button_timer_isr() {
    unsigned long interrupt_time = millis();
    if (interrupt_time - TIMERS[INTERRUPT_DEBOUNCE] > DEBOUNCE_DELAY) {
        BUTTONS_PRESSED[BUTTON_TIMER] = true;
        TIMERS[INTERRUPT_DEBOUNCE] = interrupt_time;
    }
}

void IRAM_ATTR button_wifi_isr() {
    unsigned long interrupt_time = millis();
    if (interrupt_time - TIMERS[INTERRUPT_DEBOUNCE] > DEBOUNCE_DELAY) {
        BUTTONS_PRESSED[BUTTON_WIFI] = true;
        TIMERS[INTERRUPT_DEBOUNCE] = interrupt_time;
    }
}

void IRAM_ATTR button_thema_isr() {
    unsigned long interrupt_time = millis();
    if (interrupt_time - TIMERS[INTERRUPT_DEBOUNCE] > DEBOUNCE_DELAY) {
        BUTTONS_PRESSED[BUTTON_THEMA] = true;
        TIMERS[INTERRUPT_DEBOUNCE] = interrupt_time;
    }
}

void IRAM_ATTR button_gezegde_isr() {
    unsigned long interrupt_time = millis();
    if (interrupt_time - TIMERS[INTERRUPT_DEBOUNCE] > DEBOUNCE_DELAY) {
        BUTTONS_PRESSED[BUTTON_GEZEGDE] = true;
        TIMERS[INTERRUPT_DEBOUNCE] = interrupt_time;
    }
}

void initialize_buttons() {
    pinMode(18, INPUT_PULLUP);  // DIMMER
    pinMode(19, INPUT_PULLUP);  // TIMER
    pinMode(21, INPUT_PULLUP);  // WIFI
    pinMode(22, INPUT_PULLUP);  // THEMA
    pinMode(23, INPUT_PULLUP);  // GEZEGDE

    attachInterrupt(digitalPinToInterrupt(18), button_dimmer_isr, FALLING);
    attachInterrupt(digitalPinToInterrupt(19), button_timer_isr, FALLING);
    attachInterrupt(digitalPinToInterrupt(21), button_wifi_isr, FALLING);
    attachInterrupt(digitalPinToInterrupt(22), button_thema_isr, FALLING);
    attachInterrupt(digitalPinToInterrupt(23), button_gezegde_isr, FALLING);
}
