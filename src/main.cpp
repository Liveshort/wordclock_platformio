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
int USER_SETTINGS[SETTINGS_COUNT];
byte ANIMATION_STATES[ANIMATION_COUNT];
byte CURRENT_TIME_WORDS[7];
byte TARGET_TIME_WORDS[7];
byte MINUTE_DOTS[5];
byte DRAWING_BOARD_LEDS[174];
byte DRAWING_BOARD_COLORS[174][3];  // RGB colors for each LED
int LIGHT_SENSOR_VALUES[2][10];
int RANDOM_SAYING_INDEX;
Logger LOGGER;
Storage STORAGE;
WCNetworkManager NETWORK_MANAGER;
LEDController LED_CONTROLLER;
SUPER_STATE CURR_STATE, NEXT_STATE;
NORMAL_OPERATION_SUBSTATE CURR_NO_SUBSTATE, NEXT_NO_SUBSTATE;

void initialize_globals_and_workers() {
    for (int i = 0; i < FLAG_COUNT; ++i)
        FLAGS[i] = false;

    // Initialize global strings
    STRINGS[CURRENT_TIME] = new char[10];
    STRINGS[TARGET_TIME] = new char[10];
    STRINGS[CURRENT_ROUNDED_TIME] = new char[10];
    STRINGS[TARGET_ROUNDED_TIME] = new char[10];
    STRINGS[TIMESTAMP] = new char[20];
    STRINGS[TIME_ZONE] = new char[20];
    strncpy(STRINGS[CURRENT_TIME], "--:--:--", 10);
    strncpy(STRINGS[TARGET_TIME], "--:--:--", 10);
    strncpy(STRINGS[CURRENT_ROUNDED_TIME], "--:--", 10);
    strncpy(STRINGS[TARGET_ROUNDED_TIME], "--:--", 10);
    strncpy(STRINGS[TIMESTAMP], "------ --:--:--", 20);
    strncpy(STRINGS[TIME_ZONE], "-", 20);

    for (int i = 0; i < TIMER_COUNT; ++i)
        TIMERS[i] = 0;

    for (int i = 0; i < BUTTON_COUNT; ++i)
        BUTTONS_PRESSED[i] = false;

    for (int i = 0; i < ANIMATION_COUNT; ++i)
        ANIMATION_STATES[i] = 0;
    ANIMATION_STATES[BLOCKING_FADE] = 255;

    for (int i = 0; i < 7; ++i)
        CURRENT_TIME_WORDS[i] = 255;
    for (int i = 0; i < 7; ++i)
        TARGET_TIME_WORDS[i] = 255;
    for (int i = 0; i < 5; ++i)
        MINUTE_DOTS[i] = 0;

    for (int i = 0; i < 174; ++i) {
        DRAWING_BOARD_LEDS[i] = 0;
        DRAWING_BOARD_COLORS[i][0] = 255;  // R
        DRAWING_BOARD_COLORS[i][1] = 255;  // G
        DRAWING_BOARD_COLORS[i][2] = 255;  // B
    }

    for (int i = 0; i < 2; ++i)
        for (int j = 0; j < 10; ++j)
            LIGHT_SENSOR_VALUES[i][j] = 0;

    LOGGER = Logger();
    STORAGE = Storage();
    STORAGE.initialize();
    NETWORK_MANAGER = WCNetworkManager();
    LED_CONTROLLER = LEDController();

    CURR_STATE = INITIALIZING;
    NEXT_STATE = INITIALIZING;
    CURR_NO_SUBSTATE = SUBSTATE_SHOW_TIME;
    NEXT_NO_SUBSTATE = SUBSTATE_SHOW_TIME;

    initialize_buttons();

    // Check and load user settings
    if (STORAGE.check_user_settings_saved()) {
        STORAGE.load_user_settings();
    } else {
        STORAGE.default_user_settings();
        STORAGE.load_user_settings();
    }
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

// A fast transition can interrupt a slow transition. If this happens, we need to make sure the flags are reset.
// A fast transition is used for user interactions, while a slow transition is used for automatic time updates.
// The state change to NEXT_STATE is handled in the main loop after this function is called and is immediate.
// It takes a snapshot of the current LED state for crossfading and uses that as the source for the crossfade.
void trigger_fast_transition() {
    FLAGS[TRANSITIONING] = true;
    FLAGS[TRIGGER_STATE_CHANGE] = true;
    FLAGS[CROSSFADING] = true;
    LED_CONTROLLER.save_current_state();

    FLAGS[FADING_IN] = false;
    FLAGS[FADING_OUT] = false;
}

// A slow transition cannot interrupt a fast transition, so no need to reset flags here.
// The old state is allowed to update during the fade out. The state change is triggered after fade out is complete.
void trigger_slow_transition() {
    FLAGS[TRANSITIONING] = true;
    FLAGS[FADING_OUT] = true;
}

// Light sensor array keeps track of the past 10 values of each side, so that an average can be taken.
// It uses a sliding index to overwrite the last value of each array. If it reaches the end, it starts again at the
// start.
void update_light_sensor_values() {
    static byte index = 0;

    LIGHT_SENSOR_VALUES[0][index] = analogRead(33);
    LIGHT_SENSOR_VALUES[1][index] = analogRead(32);

    Serial.println("Light levels - Left: " + String(LIGHT_SENSOR_VALUES[0][index]) +
                   " | Right: " + String(LIGHT_SENSOR_VALUES[1][index]));

    index = (index + 1) % 10;
}

void return_to_normal_operation() {
    NEXT_STATE = NORMAL_OPERATION;
    CURR_NO_SUBSTATE = SUBSTATE_SHOW_TIME;

    TIMERS[SAYING_INTERVAL_TIMER] = millis();
    trigger_slow_transition();
}

void loop() {
    // Handle background tasks
    EVERY_N_MILLISECONDS(500) {
        NETWORK_MANAGER.update();
        update_time();
        update_light_sensor_values();
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
        trigger_fast_transition();
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
                    set_current_time_to_target_time();
                    trigger_slow_transition();
                }

                LED_CONTROLLER.time_synced_blink();
                break;
            case NORMAL_OPERATION:
                switch (CURR_NO_SUBSTATE) {
                    case SUBSTATE_SHOW_TIME:
                        // Trigger a transition if the time string has changed
                        if (!FLAGS[TRANSITIONING] && !FLAGS[UPDATING_TIME_STRING] &&
                            strncmp(STRINGS[CURRENT_ROUNDED_TIME], STRINGS[TARGET_ROUNDED_TIME], 10) != 0) {
                            trigger_slow_transition();
                            FLAGS[UPDATING_TIME_STRING] = true;
                        }
                        // Update the current time string after fading out
                        if (FLAGS[UPDATING_TIME_STRING] && FLAGS[FADING_IN]) {
                            set_current_time_to_target_time();
                            FLAGS[UPDATING_TIME_STRING] = false;
                        }

                        // If the sayings timer has expired, show a saying
                        if (!FLAGS[TRANSITIONING] &&
                            (millis() - TIMERS[SAYING_INTERVAL_TIMER] > USER_SETTINGS[SAYING_INTERVAL_S] * 1000)) {
                            TIMERS[SAYING_INTERVAL_TIMER] = millis();
                            int half_fade_offset_ms = 1000 * USER_SETTINGS[FADE_CYCLE_S] * 255 / 240 / 2;
                            TIMERS[RANDOM_SAYING_TIMER] = millis() + half_fade_offset_ms;
                            NEXT_NO_SUBSTATE = SUBSTATE_SHOW_SAYING;
                            RANDOM_SAYING_INDEX =
                                random(0, 20);  // There are 11 sayings, every one is duplicated, some have 2 variants.
                                                // The 11th saying has two pieces that are displayed after another.
                            FLAGS[TWO_PART_SAYING_GO_TO_PART_2] = false;
                            trigger_slow_transition();
                        }

                        LED_CONTROLLER.show_time();
                        break;
                    case SUBSTATE_SHOW_SAYING:
                        if (!TRANSITIONING)
                            FLAGS[TWO_PART_SAYING_GO_TO_PART_2] = false;

                        // After showing the saying for 10 seconds, return to showing the time
                        if (RANDOM_SAYING_INDEX != 20 && !FLAGS[TRANSITIONING] &&
                            (millis() - TIMERS[RANDOM_SAYING_TIMER] > USER_SETTINGS[SAYING_DURATION_S] * 1000)) {
                            NEXT_NO_SUBSTATE = SUBSTATE_SHOW_TIME;
                            trigger_slow_transition();
                        } else if (RANDOM_SAYING_INDEX == 20 && !FLAGS[TRANSITIONING] &&
                                   (millis() - TIMERS[RANDOM_SAYING_TIMER] >
                                    (USER_SETTINGS[SAYING_DURATION_S] * 1000) / 2)) {
                            FLAGS[TWO_PART_SAYING_GO_TO_PART_2] = true;
                            // The index is set to 21 in the safe state change handler after the state change is
                            // triggered
                            trigger_slow_transition();
                        }

                        LED_CONTROLLER.show_saying(RANDOM_SAYING_INDEX);
                        break;
                }
                break;
            case DRAWING_BOARD:
                // Check for timeout (return to normal after 5 minutes of inactivity)
                if (millis() - TIMERS[DRAWING_BOARD_TIMER] > 300000) {
                    LOGGER.println("Drawing board timeout - returning to normal operation");
                    return_to_normal_operation();
                }

                // Display drawing board LEDs
                LED_CONTROLLER.show_drawing_board();
                break;
            default:
                break;
        }

        // If time is not being displayed, just update the current time every frame
        if (!FLAGS[TRANSITIONING] && (CURR_STATE != NORMAL_OPERATION || CURR_NO_SUBSTATE != SUBSTATE_SHOW_TIME)) {
            set_current_time_to_target_time();
        }

        // Handle overlays
        LED_CONTROLLER.overlay_AP_active(FLAGS[AP_ACTIVE]);

        // Finally write the LED changes
        LED_CONTROLLER.update();

        // State changes are only triggered when the animations are in a 'good' state, e.g. in between fading out and
        // fading in or directly when a crossfade is used. When a crossfade is used, the new state handles the complete
        // transition.
        if (FLAGS[TRIGGER_STATE_CHANGE]) {
            CURR_STATE = NEXT_STATE;
            CURR_NO_SUBSTATE = NEXT_NO_SUBSTATE;
            FLAGS[TRIGGER_STATE_CHANGE] = false;

            // Special handling for the two part saying
            if (FLAGS[TWO_PART_SAYING_GO_TO_PART_2])
                RANDOM_SAYING_INDEX = 21;
        }
    }

    delay(1);
}
