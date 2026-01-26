#ifndef _MAIN_H_
#define _MAIN_H_

#include <Arduino.h>
#include <Preferences.h>

#include "led_control.h"
#include "log.h"
#include "networking.h"
#include "persistent_storage.h"

void setup();
void loop();

enum FLAGS {
    TIME_INITIALIZED,               // True if time has been synchronized at least once
    WIFI_ACTIVE,                    // True if WiFi is enabled
    AP_ACTIVE,                      // True if Access Point is enabled
    WIFI_CONNECTING,                // True if currently trying to connect to WiFi
    WIFI_CONNECTED_F,               // True if WiFi is connected
    TRANSITIONING,                  // True if currently in a transition between states. This disables state changes.
    TRIGGER_STATE_CHANGE,           // Set to true to trigger a state change in the next loop iteration
    FADING_IN,                      // True if currently fading in (blocking)
    FADING_OUT,                     // True if currently fading out (blocking)
    CROSSFADING,                    // True if currently crossfading between two animations
    TWO_PART_SAYING_GO_TO_PART_2,   // For the two part saying, go to part 2
    UPDATING_TIME_STRING,           // True if currently updating the time string
    SERVER_REQUESTS_DRAWING_BOARD,  // Server wants to enter drawing board mode
    FLAG_COUNT
};
extern bool FLAGS[];

enum STRINGS {
    CURRENT_TIME,          // Current time as string HH:MM:SS
    TARGET_TIME,           // Target time as string HH:MM:SS
    CURRENT_ROUNDED_TIME,  // Current time as string HH:MM rounded to nearest 5 minutes
    TARGET_ROUNDED_TIME,   // Target time as string HH:MM rounded to nearest 5 minutes
    TIMESTAMP,             // Current timestamp as string DD-MMM HH:MM:SS
    TIME_ZONE,             // Current time zone as string (e.g. CEST (+0200))
    STRING_COUNT
};
extern char* STRINGS[];

enum TIMERS {
    TIME_SYNC,              // Last time synchronization
    TIME_UPDATE,            // Last time the time strings were updated
    WIFI_SCAN,              // Last WiFi scan
    WIFI_STATUS_UPDATE,     // Last WiFi status update
    WIFI_CONNECT_ATTEMPT,   // Last WiFi connect attempt
    WIFI_CONNECTED_T,       // Last time WiFi was connected
    WIFI_CONNECT_FAILED,    // Last time WiFi connection failed
    INTERRUPT_DEBOUNCE,     // Last time a button interrupt was handled
    SAYING_INTERVAL_TIMER,  // Timer for automatic saying
    RANDOM_SAYING_TIMER,    // Timer for random saying
    DRAWING_BOARD_TIMER,    // Timer for drawing board activity
    TIMER_COUNT
};
extern unsigned long TIMERS[];

enum BUTTONS {
    BUTTON_DIMMER,   // GPIO 18
    BUTTON_TIMER,    // GPIO 19
    BUTTON_WIFI,     // GPIO 21
    BUTTON_THEMA,    // GPIO 22
    BUTTON_GEZEGDE,  // GPIO 23
    BUTTON_COUNT
};
extern volatile bool BUTTONS_PRESSED[];

enum USER_SETTINGS {
    ROUND_DOWN_TIME,  // Setting this to 0 will make 12:27:30 -> 12:30 and 12:32:29 -> 12:30. Any other value will make
                      // the time always round down in increments of 5 minutes, i.e. 12:29:59 -> 12:25. This settings
                      // also influences the behavior of the minute dots.
    SAYINGS_ENABLED,  // Enables or disables sayings.
    SAYING_INTERVAL_S,   // Determines the interval at which sayings occur in seconds.
    SAYING_DURATION_S,   // Determines how long a saying is displayed in seconds.
    FADE_CYCLE_S,        // Duration of fade in/out cycles in seconds.
    ACTIVE_PALETTES,     // Bitmask of active palettes
    PALETTE_INTERVAL_S,  // Interval to change palette in seconds
    PALETTE_CYCLE_S,     // Cycle speed for return to start of palette in seconds
    SETTINGS_COUNT
};
extern int USER_SETTINGS[];

enum SUPER_STATE {
    INITIALIZING,
    WAITING_FOR_WIFI,
    WIFI_CONNECTED_S,
    WAITING_FOR_TIME_SYNC,
    TIME_SYNCED_S,
    NORMAL_OPERATION,
    DRAWING_BOARD,
    FORCED_SAYING,
    TIMER_SET,
    TIMER_RUNNING,
    TIMER_FINISHED,
    NIGHT_MODE
};
extern SUPER_STATE CURR_STATE;
extern SUPER_STATE NEXT_STATE;

enum NORMAL_OPERATION_SUBSTATE { SUBSTATE_SHOW_TIME, SUBSTATE_SHOW_SAYING };
extern NORMAL_OPERATION_SUBSTATE CURR_NO_SUBSTATE;
extern NORMAL_OPERATION_SUBSTATE NEXT_NO_SUBSTATE;

enum ANIMATIONS {
    BLOCKING_FADE,
    CROSSFADE,
    WIFI_BREATHING,
    WIFI_CONNECTED_BLINK,
    TIME_SYNC_BREATHING,
    TIME_SYNCED_BLINK,
    OVERLAY_AP_ACTIVE,
    OVERLAY_BUTTON_PRESS_1,
    OVERLAY_BUTTON_PRESS_2,
    OVERLAY_BUTTON_PRESS_3,
    OVERLAY_BUTTON_PRESS_4,
    OVERLAY_BUTTON_PRESS_5,
    ANIMATION_COUNT
};
extern byte ANIMATION_STATES[];

extern byte CURRENT_TIME_WORDS[];
extern byte TARGET_TIME_WORDS[];
extern byte MINUTE_DOTS[];
extern byte DRAWING_BOARD_LEDS[174];
extern byte DRAWING_BOARD_COLORS[174][3];

extern int LIGHT_SENSOR_VALUES[2][10];
extern int RANDOM_SAYING_INDEX;

extern Logger LOGGER;
extern Storage STORAGE;
extern WCNetworkManager NETWORK_MANAGER;
extern LEDController LED_CONTROLLER;

#endif
