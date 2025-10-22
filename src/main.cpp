#include <FastLED.h>
#include <stdint.h>

#include "main.h"
#include "buttons.h"
#include "networking.h"
#include "time_functions.h"
#include "persistent_storage.h"
#include "log.h"

#define LED_PIN     14
#define NUM_LEDS    4
#define BRIGHTNESS  255
#define LED_TYPE    WS2812
#define COLOR_ORDER GRB

bool FLAGS[FLAG_COUNT];
char * STRINGS[STRING_COUNT];
unsigned long TIMERS[TIMER_COUNT];
volatile bool BUTTONS_PRESSED[BUTTON_COUNT];
byte CURRENT_TIME_WORDS[7];
Logger LOGGER;
Storage STORAGE;
WCNetworkManager NETWORK_MANAGER;
SUPER_STATE PREV_STATE, CURR_STATE, NEXT_STATE;
NORMAL_OPERATION_SUBSTATE PREV_NO_SUBSTATE, CURR_NO_SUBSTATE, NEXT_NO_SUBSTATE;

CRGB leds[NUM_LEDS];

void initialize_globals_and_workers() {
    for (int i = 0; i < FLAG_COUNT; ++i) FLAGS[i] = false;

    // Initialize global strings
    STRINGS[TIME] = new char[10];
    STRINGS[TIMESTAMP] = new char[20];
    STRINGS[TIME_ZONE] = new char[20];
    strncpy(STRINGS[TIME], "--:--:--", 10);
    strncpy(STRINGS[TIMESTAMP], "------ --:--:--", 20);
    strncpy(STRINGS[TIME_ZONE], "-", 20);

    for (int i = 0; i < TIMER_COUNT; ++i) TIMERS[i] = 0;

    for (int i = 0; i < BUTTON_COUNT; ++i) BUTTONS_PRESSED[i] = false;

    for (int i = 0; i < 6; ++i) CURRENT_TIME_WORDS[i] = 255;

    LOGGER = Logger();
    STORAGE = Storage();
    STORAGE.initialize();
    NETWORK_MANAGER = WCNetworkManager();

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
    
    if (false && STORAGE.check_saved_wifi_credentials()) NETWORK_MANAGER.turn_on_wifi();
    else NETWORK_MANAGER.turn_on_wifi_and_AP();
    
    // This has to be after turning on the wifi, otherwise it will bootloop due to opening a socket before wifi is active
    initialize_sntp_time_servers();

    FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
    FastLED.setBrightness(BRIGHTNESS);
}

void loop() {
    static bool state = false;
    if (state) {
        leds[0] = CRGB::Gray10;
        if (FLAGS[TIME_INITIALIZED]) leds[1] = CRGB::Green;
        if (FLAGS[WIFI_ACTIVE]) leds[2] = CRGB::Blue;
        if (FLAGS[AP_ACTIVE]) leds[3] = CRGB::Red;
        state = false;
    } else {
        leds[0] = CRGB::Black;
        if (!FLAGS[TIME_INITIALIZED]) leds[1] = CRGB::Black;
        if (!FLAGS[WIFI_ACTIVE]) leds[2] = CRGB::Black;
        if (!FLAGS[AP_ACTIVE]) leds[3] = CRGB::Black;
        state = true;
    }
    FastLED.show();

    // if (BUTTONS_PRESSED[BUTTON_22]) {
    //     Serial.println("Button 22 pressed!");
    //     BUTTONS_PRESSED[BUTTON_22] = false;
    // }

    // if (BUTTONS_PRESSED[BUTTON_23]) {
    //     Serial.println("Button 23 pressed!");
    //     BUTTONS_PRESSED[BUTTON_23] = false;
    // }

    // int light_transistor_value = analogRead(4);
    // int ldr_value = analogRead(2);

    // Serial.println("Transistor: " + String(light_transistor_value) + "| LDR: " + String(ldr_value));

    NETWORK_MANAGER.update();
    update_time();

    switch(CURR_STATE) {
        case INITIALIZING:
            NEXT_STATE = WAITING_FOR_WIFI;
            break;
        case WAITING_FOR_WIFI:
            if (FLAGS[WIFI_CONNECTED_F]) {
                NEXT_STATE = WAITING_FOR_TIME_SYNC;
            } else if (!FLAGS[WIFI_ACTIVE] && !FLAGS[WIFI_CONNECTING]) {
                // Apparently connecting to wifi did not work, so color the wifi indicator LED red and do nothing
                int TODO = 0;
            }
            break;
        case WAITING_FOR_TIME_SYNC:
            if (FLAGS[TIME_INITIALIZED]) {
                NEXT_STATE = NORMAL_OPERATION;
            }
            break;
        case NORMAL_OPERATION:
            // Normal operation code here
            break;
        default:
            break;
    }

    delay(500);
}
