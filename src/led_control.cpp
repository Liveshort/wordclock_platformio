
#include <FastLED.h>

#include "led_control.h"
#include "main.h"

#define LED_PIN 13
#define BRIGHTNESS 255
#define LED_TYPE WS2812
#define COLOR_ORDER GRB
#define ANIMATION_FPS 60

#define NUM_LEDS_PHYSICAL 186
#define NUM_LEDS_LOGICAL 174
#define BREATHING_CYCLE_S 4
#define CROSSFADE_LENGTH_MS 300
#define WIFI_LED_INDEX 168

CRGB leds_source[NUM_LEDS_LOGICAL];
CRGB leds_physical[NUM_LEDS_PHYSICAL];
CRGB leds_logical[NUM_LEDS_LOGICAL];

void fade_out(bool skip_minute_dots = false) {
    // Instant fade out if duration is set to 0
    if (USER_SETTINGS[FADE_CYCLE_S] == 0) {
        ANIMATION_STATES[BLOCKING_FADE] = 0;
        FLAGS[FADING_OUT] = false;
        FLAGS[FADING_IN] = true;
        FLAGS[TRIGGER_STATE_CHANGE] = true;

        return;
    }

    int8_t direction = 255 / (ANIMATION_FPS * USER_SETTINGS[FADE_CYCLE_S] / 2);
    int8_t step = direction;

    ANIMATION_STATES[BLOCKING_FADE] -= direction;
    if (ANIMATION_STATES[BLOCKING_FADE] < step) {
        ANIMATION_STATES[BLOCKING_FADE] = 0;
        FLAGS[FADING_OUT] = false;
        FLAGS[FADING_IN] = true;
        FLAGS[TRIGGER_STATE_CHANGE] = true;
    }

    int minute_dots_subtract_index = skip_minute_dots ? 5 : 0;

    for (int i = 0; i < NUM_LEDS_LOGICAL - minute_dots_subtract_index; ++i) {
        leds_logical[i] = blend(CRGB::Black, leds_logical[i], ANIMATION_STATES[BLOCKING_FADE]);
    }
}

void fade_in(bool skip_minute_dots = false) {
    // Instant fade in if duration is set to 0
    if (USER_SETTINGS[FADE_CYCLE_S] == 0) {
        ANIMATION_STATES[BLOCKING_FADE] = 255;
        FLAGS[FADING_IN] = false;
        FLAGS[TRANSITIONING] = false;

        return;
    }

    int8_t direction = 255 / (ANIMATION_FPS * USER_SETTINGS[FADE_CYCLE_S] / 2);
    int8_t step = direction;

    ANIMATION_STATES[BLOCKING_FADE] += direction;
    if (ANIMATION_STATES[BLOCKING_FADE] > 255 - step) {
        ANIMATION_STATES[BLOCKING_FADE] = 255;
        FLAGS[FADING_IN] = false;
        FLAGS[TRANSITIONING] = false;
    }

    int minute_dots_subtract_index = skip_minute_dots ? 5 : 0;

    for (int i = 0; i < NUM_LEDS_LOGICAL - minute_dots_subtract_index; ++i) {
        leds_logical[i] = blend(CRGB::Black, leds_logical[i], ANIMATION_STATES[BLOCKING_FADE]);
    }
}

void crossfade() {
    // Instant crossfade if regular fade cycle duration is set to 0
    if (USER_SETTINGS[FADE_CYCLE_S] == 0) {
        ANIMATION_STATES[CROSSFADE] = 255;
        FLAGS[CROSSFADING] = false;
        FLAGS[TRANSITIONING] = false;
    }

    int8_t direction = 255 / (ANIMATION_FPS * CROSSFADE_LENGTH_MS / 1000);
    int8_t step = direction;

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
    int8_t direction = 255 / (ANIMATION_FPS * BREATHING_CYCLE_S / 2);
    int8_t step = direction;

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
    int8_t direction = 255 / (ANIMATION_FPS * BREATHING_CYCLE_S / 2);
    int8_t step = direction;

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
    int8_t direction = 255 / (ANIMATION_FPS * 1 / 2);
    int8_t step = direction;

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
    int8_t direction = 255 / (ANIMATION_FPS * BREATHING_CYCLE_S / 2);
    int8_t step = direction;

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
    int8_t direction = 255 / (ANIMATION_FPS * 1 / 2);
    int8_t step = direction;

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

void LEDController::show_saying(int saying_index) {
    // Clear target LEDs
    for (int i = 0; i < NUM_LEDS_LOGICAL; ++i) {
        leds_logical[i] = CRGB::Black;
    }

    // Set target LEDs based on the saying index
    for (int j = 0; j < 7; ++j) {
        byte word = SAYINGS[saying_index][j];

        if (word == 255)
            break;

        for (int k = 0; k < 7; ++k) {
            byte led_index = W2LL[word][k];

            if (led_index == 255)
                break;

            leds_logical[led_index] = CRGB::White;
        }
    }

    // The minute dots should start all bright and slowly fade one by one to off from right to left
    // First calculate the permyriads (0 to 10000) of the saying duration that has passed
    int quarter_fade_offset_ms = 1000 * USER_SETTINGS[FADE_CYCLE_S] * 255 / 240 / 4;
    int bips_elapsed = 10000 * ((long) (millis() - TIMERS[RANDOM_SAYING_TIMER]) - quarter_fade_offset_ms) /
                       (USER_SETTINGS[SAYING_DURATION_S] * 1000);
    if (bips_elapsed < 0)
        bips_elapsed = 0;
    else if (bips_elapsed > 10000)
        bips_elapsed = 10000;

    int full_dots = (10000 - bips_elapsed) / 2000;               // Each dot represents 2000 permyriads
    int partial_dot_permyriads = (10000 - bips_elapsed) % 2000;  // Remainder for the partial dot

    for (int i = 0; i < 5; i++) {
        if (i < full_dots) {
            leds_logical[169 + i] = CRGB::White;  // Fully on
        } else if (i == full_dots) {
            leds_logical[169 + i] =
                blend(CRGB::Black, CRGB::White, partial_dot_permyriads * 255 / 2000);  // Partial brightness
        } else {
            leds_logical[169 + i] = CRGB::Black;  // Fully off
        }
    }

    if (FLAGS[FADING_OUT]) {
        if (FLAGS[TWO_PART_SAYING_GO_TO_PART_2])
            fade_out(true);  // keep the minute dots on during the transition to part 2
        else
            fade_out();
    } else if (FLAGS[FADING_IN]) {
        if (FLAGS[TWO_PART_SAYING_GO_TO_PART_2])
            fade_in(true);  // keep the minute dots on during the transition to part 2
        else
            fade_in();
    } else if (FLAGS[CROSSFADING])
        crossfade();
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

void LEDController::update() {
    // Map logical LED array to physical LEDs
    for (int i = 0; i < NUM_LEDS_PHYSICAL; i++)
        leds_physical[i] = CRGB::Black;
    for (int i = 0; i < NUM_LEDS_LOGICAL; i++)
        leds_physical[L2P[i]] = leds_logical[i];

    // Set the overall brightness based on the average light level
    int left_light_average = 0;
    int right_light_average = 0;
    for (int i = 0; i < 10; ++i) {
        left_light_average += LIGHT_SENSOR_VALUES[0][i];
        right_light_average += LIGHT_SENSOR_VALUES[1][i];
    }

    left_light_average /= 10;
    right_light_average /= 10;

    int max_light_average = min(left_light_average, right_light_average);

    for (int i = 0; i < NUM_LEDS_PHYSICAL; ++i)
        leds_physical[i].fadeToBlackBy(max_light_average / 16);

    // Apply gamma correction based on average light level, this makes the brightness levels appear more linear to the
    // human eye
    for (int i = 0; i < NUM_LEDS_PHYSICAL; i++)
        leds_physical[i].fadeToBlackBy(
            GAMMA_CORRECTION_PERCENTAGE[max(leds_physical[i].r, max(leds_physical[i].g, leds_physical[i].b))]);

    FastLED.show();
}
