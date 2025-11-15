#include <FastLED.h>
#include <stdint.h>

#include "buttons.h"
#include "led_control.h"
#include "log.h"
#include "main.h"
#include "networking.h"
#include "persistent_storage.h"
#include "time_functions.h"

bool FLAGS[FLAG_COUNT];
char* STRINGS[STRING_COUNT];
unsigned long TIMERS[TIMER_COUNT];
volatile bool BUTTONS_PRESSED[BUTTON_COUNT];
byte ANIMATION_STATES[ANIMATION_COUNT];
byte CURRENT_TIME_WORDS[7];
byte TARGET_TIME_WORDS[7];
byte MINUTE_DOTS = 0;
byte DRAWING_BOARD_LEDS[174];
byte DRAWING_BOARD_COLORS[174][3];  // RGB colors for each LED
Logger LOGGER;
Storage STORAGE;
WCNetworkManager NETWORK_MANAGER;
LEDController LED_CONTROLLER;
SUPER_STATE PREV_STATE, CURR_STATE, NEXT_STATE;
NORMAL_OPERATION_SUBSTATE PREV_NO_SUBSTATE, CURR_NO_SUBSTATE, NEXT_NO_SUBSTATE;

void initialize_globals_and_workers() {
    for (int i = 0; i < FLAG_COUNT; ++i)
        FLAGS[i] = false;

    // Initialize global strings

    STRINGS[CURRENT_TIME] = new char[10];
    STRINGS[TARGET_TIME] = new char[10];
    STRINGS[TIMESTAMP] = new char[20];
    STRINGS[TIME_ZONE] = new char[20];
    strncpy(STRINGS[CURRENT_TIME], "--:--:--", 10);
    strncpy(STRINGS[TARGET_TIME], "--:--:--", 10);
    strncpy(STRINGS[TIMESTAMP], "------ --:--:--", 20);
    strncpy(STRINGS[TIME_ZONE], "-", 20);

    for (int i = 0; i < TIMER_COUNT; ++i)
        TIMERS[i] = 0;

    for (int i = 0; i < BUTTON_COUNT; ++i)
        BUTTONS_PRESSED[i] = false;

    for (int i = 0; i < ANIMATION_COUNT; ++i)
        ANIMATION_STATES[i] = 0;
    ANIMATION_STATES[BLOCKING_FADE] = 255;

    for (int i = 0; i < 6; ++i)
        CURRENT_TIME_WORDS[i] = 255;
    for (int i = 0; i < 6; ++i)
        TARGET_TIME_WORDS[i] = 255;

    for (int i = 0; i < 174; ++i) {
        DRAWING_BOARD_LEDS[i] = 0;
        DRAWING_BOARD_COLORS[i][0] = 255;  // R
        DRAWING_BOARD_COLORS[i][1] = 255;  // G
        DRAWING_BOARD_COLORS[i][2] = 255;  // B
    }

    LOGGER = Logger();
    STORAGE = Storage();
    STORAGE.initialize();
    NETWORK_MANAGER = WCNetworkManager();
    LED_CONTROLLER = LEDController();

    PREV_STATE = INITIALIZING;
    CURR_STATE = INITIALIZING;
    NEXT_STATE = INITIALIZING;
    PREV_NO_SUBSTATE = SUBSTATE_SHOW_TIME;
    CURR_NO_SUBSTATE = SUBSTATE_SHOW_TIME;
    NEXT_NO_SUBSTATE = SUBSTATE_SHOW_TIME;

    initialize_buttons();
}

void setup() {
    Serial.begin(115200);

    initialize_globals_and_workers();

    if (false && STORAGE.check_saved_wifi_credentials())
        NETWORK_MANAGER.turn_on_wifi();
    else
        NETWORK_MANAGER.turn_on_wifi_and_AP();

    // This has to be after turning on the wifi, otherwise it will bootloop due to opening a socket before wifi is
    // active
    initialize_sntp_time_servers();

    LED_CONTROLLER.initialize_led_controller();
}

void loop() {
    // int light_transistor_value = analogRead(4);
    // int ldr_value = analogRead(2);

    // Serial.println("Transistor: " + String(light_transistor_value) + "| LDR: " + String(ldr_value));

    EVERY_N_MILLISECONDS(500) {
        NETWORK_MANAGER.update();
        update_time();

        // Temporarily set current time to target time for testing
        set_current_time_to_target_time();
    }

    // Check for button presses
    if (BUTTONS_PRESSED[BUTTON_DIMMER]) {
        BUTTONS_PRESSED[BUTTON_DIMMER] = false;
        LOGGER.println("DIMMER button pressed!");
    }
    if (BUTTONS_PRESSED[BUTTON_TIMER]) {
        BUTTONS_PRESSED[BUTTON_TIMER] = false;
        LOGGER.println("TIMER button pressed!");
    }
    if (BUTTONS_PRESSED[BUTTON_WIFI]) {
        BUTTONS_PRESSED[BUTTON_WIFI] = false;
        LOGGER.println("WIFI button pressed!");
    }
    if (BUTTONS_PRESSED[BUTTON_THEMA]) {
        BUTTONS_PRESSED[BUTTON_THEMA] = false;
        LOGGER.println("THEMA button pressed!");
    }
    if (BUTTONS_PRESSED[BUTTON_GEZEGDE]) {
        BUTTONS_PRESSED[BUTTON_GEZEGDE] = false;
        LOGGER.println("GEZEGDE button pressed!");
    }

    // Handle server-requested state changes
    if (FLAGS[SERVER_REQUESTS_DRAWING_BOARD] && CURR_STATE == NORMAL_OPERATION) {
        FLAGS[SERVER_REQUESTS_DRAWING_BOARD] = false;
        NEXT_STATE = DRAWING_BOARD;
        FLAGS[TRANSITIONING] = true;
        FLAGS[TRIGGER_STATE_CHANGE] = true;
        FLAGS[CROSSFADING] = true;
        LED_CONTROLLER.setup_crossfade();
        TIMERS[DRAWING_BOARD_TIMER] = millis();
        LOGGER.println("Server requested drawing board mode");
    }

    // Try to keep the LED updates at ~60 FPS
    EVERY_N_MILLISECONDS(17) {
        switch (CURR_STATE) {
            case INITIALIZING:
                NEXT_STATE = WAITING_FOR_WIFI;
                FLAGS[TRIGGER_STATE_CHANGE] = true;
                break;
            case WAITING_FOR_WIFI:
                if (FLAGS[WIFI_CONNECTED_F]) {
                    NEXT_STATE = WIFI_CONNECTED_S;
                    FLAGS[TRIGGER_STATE_CHANGE] = true;
                    FLAGS[TRANSITIONING] = true;
                } else if (!FLAGS[WIFI_ACTIVE] && !FLAGS[WIFI_CONNECTING]) {
                    LED_CONTROLLER.waiting_for_wifi_failed();
                }

                LED_CONTROLLER.waiting_for_wifi_breathing_animation();
                break;
            case WIFI_CONNECTED_S:
                if (!FLAGS[TRANSITIONING]) {
                    NEXT_STATE = WAITING_FOR_TIME_SYNC;
                    FLAGS[TRIGGER_STATE_CHANGE] = true;
                }

                LED_CONTROLLER.wifi_connected_blink();
                break;
            case WAITING_FOR_TIME_SYNC:
                if (FLAGS[TIME_INITIALIZED]) {
                    NEXT_STATE = TIME_SYNCED_S;
                    FLAGS[TRIGGER_STATE_CHANGE] = true;
                    FLAGS[TRANSITIONING] = true;
                }

                LED_CONTROLLER.waiting_for_time_breathing_animation();
                break;
            case TIME_SYNCED_S:
                if (!FLAGS[TRANSITIONING]) {
                    NEXT_STATE = NORMAL_OPERATION;
                    FLAGS[TRANSITIONING] = true;
                    FLAGS[FADING_OUT] = true;
                }

                LED_CONTROLLER.time_synced_blink();
                break;
            case NORMAL_OPERATION:
                // Trigger a transition if the time string has changed
                if (!FLAGS[UPDATING_TIME_STRING] && strncmp(STRINGS[CURRENT_TIME], STRINGS[TARGET_TIME], 10) != 0) {
                    FLAGS[TRANSITIONING] = true;
                    FLAGS[UPDATING_TIME_STRING] = true;
                    FLAGS[FADING_OUT] = true;
                }
                // Update the current time string after fading out
                if (FLAGS[UPDATING_TIME_STRING] && FLAGS[FADING_IN]) {
                    set_current_time_to_target_time();
                    FLAGS[UPDATING_TIME_STRING] = false;
                }

                LED_CONTROLLER.show_time();
                break;
            case DRAWING_BOARD:
                // Check for timeout (return to normal after 5 minutes of inactivity)
                if (millis() - TIMERS[DRAWING_BOARD_TIMER] > 300000) {
                    LOGGER.println("Drawing board timeout - returning to normal operation");
                    NEXT_STATE = NORMAL_OPERATION;
                    FLAGS[TRANSITIONING] = true;
                    FLAGS[FADING_OUT] = true;
                }

                // Display drawing board LEDs
                LED_CONTROLLER.show_drawing_board();
                break;
            default:
                break;
        }

        // If time is not being displayed, just update it every frame
        if (!FLAGS[TRANSITIONING] && (CURR_STATE != NORMAL_OPERATION || CURR_NO_SUBSTATE != SUBSTATE_SHOW_TIME)) {
            set_current_time_to_target_time();
        }

        // Handle overlays
        LED_CONTROLLER.overlay_AP_active(FLAGS[AP_ACTIVE]);

        // Finally write the LED changes
        LED_CONTROLLER.update();

        if (FLAGS[TRIGGER_STATE_CHANGE]) {
            PREV_STATE = CURR_STATE;
            CURR_STATE = NEXT_STATE;
            FLAGS[TRIGGER_STATE_CHANGE] = false;
        }
    }

    delay(1);
}
