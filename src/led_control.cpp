
#include <FastLED.h>

#include "led_control.h"
#include "main.h"

#define LED_PIN 13
#define BRIGHTNESS 128
#define LED_TYPE WS2812
#define COLOR_ORDER GRB
#define ANIMATION_FPS 60

#define NUM_LEDS_PHYSICAL 186
#define NUM_LEDS_LOGICAL 174
#define BREATHING_CYCLE_S 4
#define FADE_CYCLE_S 4
#define CROSSFADE_LENGTH_MS 300
#define WIFI_LED_INDEX 168

CRGB leds_source[NUM_LEDS_LOGICAL];
CRGB leds_physical[NUM_LEDS_PHYSICAL];
CRGB leds_logical[NUM_LEDS_LOGICAL];

void fade_out() {
    static int8_t direction = 255 / (ANIMATION_FPS * FADE_CYCLE_S / 2);
    static int8_t step = direction;

    ANIMATION_STATES[BLOCKING_FADE] -= direction;
    if (ANIMATION_STATES[BLOCKING_FADE] < step) {
        ANIMATION_STATES[BLOCKING_FADE] = 0;
        FLAGS[FADING_OUT] = false;
        FLAGS[FADING_IN] = true;
        FLAGS[TRIGGER_STATE_CHANGE] = true;
    }
    for (int i = 0; i < NUM_LEDS_LOGICAL; ++i) {
        leds_logical[i] = blend(CRGB::Black, leds_logical[i], ANIMATION_STATES[BLOCKING_FADE]);
    }
}

void fade_in() {
    static int8_t direction = 255 / (ANIMATION_FPS * FADE_CYCLE_S / 2);
    static int8_t step = direction;

    ANIMATION_STATES[BLOCKING_FADE] += direction;
    if (ANIMATION_STATES[BLOCKING_FADE] > 255 - step) {
        ANIMATION_STATES[BLOCKING_FADE] = 255;
        FLAGS[FADING_IN] = false;
        FLAGS[TRANSITIONING] = false;
    }
    for (int i = 0; i < NUM_LEDS_LOGICAL; ++i) {
        leds_logical[i] = blend(CRGB::Black, leds_logical[i], ANIMATION_STATES[BLOCKING_FADE]);
    }
}

void crossfade() {
    static int8_t direction = 255 / (ANIMATION_FPS * CROSSFADE_LENGTH_MS / 1000);
    static int8_t step = direction;

    ANIMATION_STATES[CROSSFADE] += direction;
    if (ANIMATION_STATES[CROSSFADE] > 255 - step) {
        ANIMATION_STATES[CROSSFADE] = 255;
        FLAGS[CROSSFADING] = false;
        FLAGS[TRANSITIONING] = false;
    }
    for (int i = 0; i < NUM_LEDS_LOGICAL; ++i) {
        leds_logical[i] = blend(leds_source[i], leds_logical[i], ANIMATION_STATES[CROSSFADE]);
    }
}

void LEDController::initialize_led_controller() {
    FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds_physical, NUM_LEDS_PHYSICAL).setCorrection(TypicalLEDStrip);
    FastLED.setBrightness(BRIGHTNESS);
}

void LEDController::save_current_state() {
    ANIMATION_STATES[CROSSFADE] = 0;
    for (int i = 0; i < NUM_LEDS_LOGICAL; ++i) {
        leds_source[i] = leds_logical[i];
    }
}

void LEDController::overlay_AP_active(bool ap_active) {
    static int8_t direction = 255 / (ANIMATION_FPS * BREATHING_CYCLE_S / 2);
    static int8_t step = direction;

    if (!ap_active)
        direction = -abs(direction);
    else
        direction = abs(direction);

    // Fade the AP active overlay in or out
    if (ANIMATION_STATES[OVERLAY_AP_ACTIVE] < step && direction < 0)
        ANIMATION_STATES[OVERLAY_AP_ACTIVE] = 0;
    else if (ANIMATION_STATES[OVERLAY_AP_ACTIVE] > 255 - step && direction > 0)
        ANIMATION_STATES[OVERLAY_AP_ACTIVE] = 255;
    else
        ANIMATION_STATES[OVERLAY_AP_ACTIVE] += direction;

    leds_logical[WIFI_LED_INDEX] = blend(leds_logical[WIFI_LED_INDEX], CRGB(0, 0, ANIMATION_STATES[OVERLAY_AP_ACTIVE]),
                                         ANIMATION_STATES[OVERLAY_AP_ACTIVE]);
}

void LEDController::waiting_for_wifi_breathing_animation() {
    static int8_t direction = 255 / (ANIMATION_FPS * BREATHING_CYCLE_S / 2);
    static int8_t step = direction;

    // Clear all LEDs
    for (int i = 0; i < NUM_LEDS_LOGICAL; i++)
        leds_logical[i] = CRGB::Black;

    // Breathing animation for the wifi LED
    ANIMATION_STATES[WIFI_BREATHING] += direction;
    if (ANIMATION_STATES[WIFI_BREATHING] < step || ANIMATION_STATES[WIFI_BREATHING] > 255 - step)
        direction = -direction;
    leds_logical[WIFI_LED_INDEX] =
        CRGB(ANIMATION_STATES[WIFI_BREATHING], ANIMATION_STATES[WIFI_BREATHING], ANIMATION_STATES[WIFI_BREATHING]);
}

void LEDController::waiting_for_wifi_failed() {
    for (int i = 0; i < NUM_LEDS_LOGICAL; i++)
        leds_logical[i] = CRGB::Black;
    leds_logical[WIFI_LED_INDEX] = CRGB::Red;
}

void LEDController::wifi_connected_blink() {
    static int8_t direction = 255 / (ANIMATION_FPS * 1 / 2);
    static int8_t step = direction;

    // Clear all LEDs
    for (int i = 0; i < NUM_LEDS_LOGICAL; i++)
        leds_logical[i] = CRGB::Black;

    // Blink the wifi LED
    ANIMATION_STATES[WIFI_CONNECTED_BLINK] += direction;
    if (ANIMATION_STATES[WIFI_CONNECTED_BLINK] >= 128)
        FLAGS[TRANSITIONING] = false;
    if ((ANIMATION_STATES[WIFI_CONNECTED_BLINK] / 32) % 2 == 0)
        leds_logical[WIFI_LED_INDEX] = CRGB::Black;
    else
        leds_logical[WIFI_LED_INDEX] = CRGB::White;
}

void LEDController::waiting_for_time_breathing_animation() {
    static int8_t direction = 255 / (ANIMATION_FPS * BREATHING_CYCLE_S / 2);
    static int8_t step = direction;

    // Clear all LEDs
    for (int i = 0; i < NUM_LEDS_LOGICAL; i++)
        leds_logical[i] = CRGB::Black;

    // Set the wifi LED to solid white after the blink
    leds_logical[WIFI_LED_INDEX] = CRGB::White;

    // Breathing animation for the time sync LED
    ANIMATION_STATES[TIME_SYNC_BREATHING] += direction;
    if (ANIMATION_STATES[TIME_SYNC_BREATHING] < step || ANIMATION_STATES[TIME_SYNC_BREATHING] > 255 - step)
        direction = -direction;

    for (int i = 0; i < 4; ++i)
        leds_logical[W2LL[TIJD][i]] = CRGB(ANIMATION_STATES[TIME_SYNC_BREATHING], ANIMATION_STATES[TIME_SYNC_BREATHING],
                                           ANIMATION_STATES[TIME_SYNC_BREATHING]);
}

void LEDController::time_synced_blink() {
    static int8_t direction = 255 / (ANIMATION_FPS * 1 / 2);
    static int8_t step = direction;

    // Clear all LEDs
    for (int i = 0; i < NUM_LEDS_LOGICAL; i++)
        leds_logical[i] = CRGB::Black;
    // Set the wifi LED to solid white after the blink
    leds_logical[WIFI_LED_INDEX] = CRGB::White;

    // Blink the time LEDs
    if (ANIMATION_STATES[TIME_SYNCED_BLINK] < 128)
        ANIMATION_STATES[TIME_SYNCED_BLINK] += direction;
    if (ANIMATION_STATES[TIME_SYNCED_BLINK] >= 128 && !FLAGS[FADING_OUT] && !FLAGS[FADING_IN])
        FLAGS[TRANSITIONING] = false;
    if (((ANIMATION_STATES[TIME_SYNCED_BLINK] / 32) % 2 == 0) && ANIMATION_STATES[TIME_SYNCED_BLINK] < 128)
        for (int i = 0; i < 4; ++i)
            leds_logical[W2LL[TIJD][i]] = CRGB::Black;
    else
        for (int i = 0; i < 4; ++i)
            leds_logical[W2LL[TIJD][i]] = CRGB::White;

    if (FLAGS[FADING_OUT])
        fade_out();
    else if (FLAGS[FADING_IN])
        fade_in();
    else if (FLAGS[CROSSFADING])
        crossfade();
}

void LEDController::show_time() {
    static int8_t direction = 255 / (ANIMATION_FPS * 1 / 2);
    static int8_t step = direction;

    // Clear target LEDs
    for (int i = 0; i < NUM_LEDS_LOGICAL; ++i) {
        leds_logical[i] = CRGB::Black;
    }

    // Set target LEDs based on current time words
    for (int i = 0; i < 7; i++) {
        if (CURRENT_TIME_WORDS[i] == 255)
            continue;

        for (int j = 0; j < 7; ++j) {
            byte led_index = W2LL[CURRENT_TIME_WORDS[i]][j];

            if (led_index == 255)
                break;

            leds_logical[led_index] = CRGB::White;
        }
    }

    // Light up the minute dots, blend based on how many minute dots are set
    for (int i = 0; i < 5; i++)
        leds_logical[169 + i] = blend(CRGB::Black, CRGB::White, MINUTE_DOTS[i]);

    if (FLAGS[FADING_OUT])
        fade_out();
    else if (FLAGS[FADING_IN])
        fade_in();
    else if (FLAGS[CROSSFADING])
        crossfade();
}

void LEDController::update() {
    // Map logical LED array to physical LEDs
    for (int i = 0; i < NUM_LEDS_PHYSICAL; i++)
        leds_physical[i] = CRGB::Black;
    for (int i = 0; i < NUM_LEDS_LOGICAL; i++)
        leds_physical[L2P[i]] = leds_logical[i];

    // Apply gamma correction based on average light level, this makes the brightness levels appear more linear to the
    // human eye
    for (int i = 0; i < NUM_LEDS_PHYSICAL; i++)
        leds_physical[i].fadeToBlackBy(
            GAMMA_CORRECTION_PERCENTAGE[max(leds_physical[i].r, max(leds_physical[i].g, leds_physical[i].b))]);

    FastLED.show();
}

void LEDController::show_drawing_board() {
    // Display LEDs based on DRAWING_BOARD_LEDS array with colors
    for (int i = 0; i < NUM_LEDS_LOGICAL; i++) {
        if (DRAWING_BOARD_LEDS[i] == 1) {
            leds_logical[i] = CRGB(DRAWING_BOARD_COLORS[i][0], DRAWING_BOARD_COLORS[i][1], DRAWING_BOARD_COLORS[i][2]);
        } else {
            leds_logical[i] = CRGB::Black;
        }
    }
}
