#ifndef _MAIN_H_
#define _MAIN_H_

#include <Preferences.h>

#include "log.h"
#include "persistent_storage.h"
#include "networking.h"

void setup();
void loop();

enum FLAGS {
    TIME_INITIALIZED,   // True if time has been synchronized at least once
    WIFI_ACTIVE,        // True if WiFi is enabled
    AP_ACTIVE,          // True if Access Point is enabled
    WIFI_CONNECTING,    // True if currently trying to connect to WiFi
    WIFI_CONNECTED_F,   // True if WiFi is connected
    ROUND_DOWN_TIME,    // Setting this to false will make 12:27:30 -> 12:30 and 12:32:29 -> 12:30. true will make it 12:29:59 -> 12:25.
    FLAG_COUNT
};
extern bool FLAGS[];

enum STRINGS {
    TIME,               // Current time as string HH:MM:SS
    TIMESTAMP,          // Current timestamp as string DD-MMM HH:MM:SS
    TIME_ZONE,          // Current time zone as string (e.g. CEST (+0200))
    STRING_COUNT
};
extern char * STRINGS[];

enum TIMERS {
    TIME_SYNC,              // Last time synchronization
    TIME_UPDATE,            // Last time the time strings were updated
    WIFI_SCAN,              // Last WiFi scan
    WIFI_STATUS_UPDATE,     // Last WiFi status update
    WIFI_CONNECT_ATTEMPT,   // Last WiFi connect attempt
    WIFI_CONNECTED_T,       // Last time WiFi was connected
    WIFI_CONNECT_FAILED,    // Last time WiFi connection failed
    INTERRUPT_DEBOUNCE,     // Last time a button interrupt was handled
    TIMER_COUNT
};
extern unsigned long TIMERS[];

enum BUTTONS {
    BUTTON_22,
    BUTTON_23,
    BUTTON_COUNT
};
extern volatile bool BUTTONS_PRESSED[];

enum SUPER_STATE {
    INITIALIZING,
    WAITING_FOR_WIFI,
    WAITING_FOR_TIME_SYNC,
    NORMAL_OPERATION,
    FORCED_SAYING,
    TIMER_SET,
    TIMER_RUNNING,
    TIMER_FINISHED,
    NIGHT_MODE
};
extern SUPER_STATE PREV_STATE;
extern SUPER_STATE CURR_STATE;
extern SUPER_STATE NEXT_STATE;

enum NORMAL_OPERATION_SUBSTATE {
    SUBSTATE_SHOW_TIME,
    SUBSTATE_SHOW_SAYING,
    SUBSTATE_TRANSITION
};
extern NORMAL_OPERATION_SUBSTATE PREV_NO_SUBSTATE;
extern NORMAL_OPERATION_SUBSTATE CURR_NO_SUBSTATE;
extern NORMAL_OPERATION_SUBSTATE NEXT_NO_SUBSTATE;

extern byte CURRENT_TIME_WORDS[];

extern Logger LOGGER;
extern Storage STORAGE;
extern WCNetworkManager NETWORK_MANAGER;

#endif
