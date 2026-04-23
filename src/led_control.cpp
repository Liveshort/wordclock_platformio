
#include <FastLED.h>

#include "led_control.h"
#include "main.h"
#include "palettes.h"

#define LED_PIN 13
#define BRIGHTNESS 255
#define LED_TYPE WS2812
#define COLOR_ORDER GRB
#define ANIMATION_FPS 60

#define NUM_LEDS_PHYSICAL 186
#define NUM_LEDS_LOGICAL 174
#define BREATHING_CYCLE_S 4
#define SELECT_CYCLE_S 2
#define CROSSFADE_LENGTH_MS 300
#define WIFI_LED_INDEX 168

uint64_t frame_counter = 0;
uint32_t reduced_frame_counter = 0;
int horizontal_scroll_frame_delay = 10;

CRGB leds_source[NUM_LEDS_LOGICAL];
CRGB leds_physical[NUM_LEDS_PHYSICAL];
CRGB leds_logical[NUM_LEDS_LOGICAL];

byte CURRENT_PALETTE_IDX;
byte TARGET_PALETTE_IDX;

void fade_out(bool skip_minute_dots = false) {
    // Instant fade out if duration is set to 0 or brightness is very low
    if (USER_SETTINGS[FADE_CYCLE_S] == 0 || USER_SETTINGS[MANUAL_BRIGHTNESS] <= 64) {
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
    // Instant fade in if duration is set to 0 or brightness is very low
    if (USER_SETTINGS[FADE_CYCLE_S] == 0 || USER_SETTINGS[MANUAL_BRIGHTNESS] <= 64) {
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
    // Instant crossfade if regular fade cycle duration is set to 0 or brightness is very low
    if (USER_SETTINGS[FADE_CYCLE_S] == 0 || USER_SETTINGS[MANUAL_BRIGHTNESS] <= 64) {
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

CRGB get_palette_row_color(byte led_index) {
    // Calculate the palette index, adding a row offset based on the LED index
    int32_t palette_index = reduced_frame_counter + USER_SETTINGS[PALETTE_ROW_SPACING] * LEDS_ROW[led_index];
    if (palette_index > 255)
        palette_index -= 256;

    return ColorFromPalette(PALETTES[CURRENT_PALETTE_IDX].pal, palette_index);
}

void LEDController::initialize_led_controller() {
    FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds_physical, NUM_LEDS_PHYSICAL).setCorrection(TypicalLEDStrip);
    FastLED.setBrightness(BRIGHTNESS);

    // If no palettes are active, just show the white palette (index 0)
    if (USER_SETTINGS[ACTIVE_PALETTES] == 0) {
        TIMERS[PALETTE_INTERVAL_TIMER] = millis();
        CURRENT_PALETTE_IDX = 0;
        TARGET_PALETTE_IDX = 0;

        return;
    }

    // If there are active palettes, set the target palette to the first active one
    TARGET_PALETTE_IDX = 0;
    while (!(USER_SETTINGS[ACTIVE_PALETTES] & (1 << TARGET_PALETTE_IDX))) {
        TARGET_PALETTE_IDX = (TARGET_PALETTE_IDX + 1) % PALETTE_COUNT;
    }

    CURRENT_PALETTE_IDX = TARGET_PALETTE_IDX;
}

void LEDController::save_current_state() {
    ANIMATION_STATES[CROSSFADE] = 0;
    for (int i = 0; i < NUM_LEDS_LOGICAL; ++i) {
        leds_source[i] = leds_logical[i];
    }
}

void LEDController::check_cycle_themes() {
    // If no palettes are active, just show the white palette (index 0)
    if (USER_SETTINGS[ACTIVE_PALETTES] == 0) {
        TIMERS[PALETTE_INTERVAL_TIMER] = millis();
        CURRENT_PALETTE_IDX = 0;
        TARGET_PALETTE_IDX = 0;

        return;
    }

    // Cycle to the next active palette
    if (millis() - TIMERS[PALETTE_INTERVAL_TIMER] > USER_SETTINGS[PALETTE_INTERVAL_S] * 1000) {
        do {
            TARGET_PALETTE_IDX = (TARGET_PALETTE_IDX + 1) % PALETTE_COUNT;
        } while (!(USER_SETTINGS[ACTIVE_PALETTES] & (1 << TARGET_PALETTE_IDX)));

        TIMERS[PALETTE_INTERVAL_TIMER] = millis();
    }
}

int LEDController::cycle_theme_now() {
    // If no palettes are active, just show the white palette (index 0)
    if (USER_SETTINGS[ACTIVE_PALETTES] == 0) {
        TIMERS[PALETTE_INTERVAL_TIMER] = millis();
        TARGET_PALETTE_IDX = 0;

        if (CURRENT_PALETTE_IDX != TARGET_PALETTE_IDX) {
            FLAGS[INTERRUPT_PALETTE_CYCLE] = true;
            return 1;
        } else {
            return 0;
        }
    }

    // Cycle to the next active palette
    do {
        TARGET_PALETTE_IDX = (TARGET_PALETTE_IDX + 1) % PALETTE_COUNT;
    } while (!(USER_SETTINGS[ACTIVE_PALETTES] & (1 << TARGET_PALETTE_IDX)));

    TIMERS[PALETTE_INTERVAL_TIMER] = millis();

    // We want to cycle right now, so set the interrupt flag if the target palette is different from the current one
    // Actual transition will be handled in the main loop
    if (CURRENT_PALETTE_IDX != TARGET_PALETTE_IDX) {
        FLAGS[INTERRUPT_PALETTE_CYCLE] = true;
        CURRENT_PALETTE_IDX = TARGET_PALETTE_IDX;
        return 1;
    }

    return 0;
}

void LEDController::overlay_wifi_active() {
    int8_t direction = 255 / (ANIMATION_FPS * BREATHING_CYCLE_S / 2);
    int8_t step = direction;

    if (!FLAGS[WIFI_ACTIVE])
        direction = -abs(direction);
    else
        direction = abs(direction);

    // Fade the AP active overlay in or out
    if (ANIMATION_STATES[OVERLAY_WIFI_ACTIVE] < step && direction < 0)
        ANIMATION_STATES[OVERLAY_WIFI_ACTIVE] = 0;
    else if (ANIMATION_STATES[OVERLAY_WIFI_ACTIVE] > 255 - step && direction > 0)
        ANIMATION_STATES[OVERLAY_WIFI_ACTIVE] = 255;
    else
        ANIMATION_STATES[OVERLAY_WIFI_ACTIVE] += direction;

    leds_logical[WIFI_LED_INDEX] = blend(leds_logical[WIFI_LED_INDEX], get_palette_row_color(WIFI_LED_INDEX),
                                         ANIMATION_STATES[OVERLAY_WIFI_ACTIVE]);
}

void LEDController::overlay_AP_active() {
    int8_t direction = 255 / (ANIMATION_FPS * BREATHING_CYCLE_S / 2);
    int8_t step = direction;

    if (!FLAGS[AP_ACTIVE])
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

void LEDController::overlay_wifi_connect_failed() {
    int8_t direction = 255 / (ANIMATION_FPS * BREATHING_CYCLE_S / 2);
    int8_t step = direction;

    if (TIMERS[WIFI_CONNECTED_T] > TIMERS[WIFI_CONNECT_FAILED])
        direction = -abs(direction);
    else
        direction = abs(direction);

    // Fade the AP active overlay in or out
    if (ANIMATION_STATES[OVERLAY_WIFI_CONNECT_FAILED] < step && direction < 0)
        ANIMATION_STATES[OVERLAY_WIFI_CONNECT_FAILED] = 0;
    else if (ANIMATION_STATES[OVERLAY_WIFI_CONNECT_FAILED] > 255 - step && direction > 0)
        ANIMATION_STATES[OVERLAY_WIFI_CONNECT_FAILED] = 255;
    else
        ANIMATION_STATES[OVERLAY_WIFI_CONNECT_FAILED] += direction;

    leds_logical[WIFI_LED_INDEX] =
        blend(leds_logical[WIFI_LED_INDEX], CRGB(ANIMATION_STATES[OVERLAY_WIFI_CONNECT_FAILED], 0, 0),
              ANIMATION_STATES[OVERLAY_WIFI_CONNECT_FAILED]);
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

            leds_logical[led_index] = get_palette_row_color(led_index);
        }
    }

    // Light up the minute dots, blend based on how many minute dots are set
    for (int i = 0; i < 5; i++)
        leds_logical[169 + i] = blend(CRGB::Black, get_palette_row_color(169 + i), MINUTE_DOTS[i]);

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

            leds_logical[led_index] = get_palette_row_color(led_index);
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
            leds_logical[169 + i] = get_palette_row_color(169 + i);  // Fully on
        } else if (i == full_dots) {
            leds_logical[169 + i] = blend(CRGB::Black, get_palette_row_color(169 + i),
                                          partial_dot_permyriads * 255 / 2000);  // Partial brightness
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

// Determines the display width of a word in number of columns
int determine_word_display_width(const char* word) {
    int width = 0;
    int len = strlen(word);

    for (int i = 0; i < len; ++i) {
        char c = toupper((unsigned char) word[i]);

        if (c >= 'A' && c <= 'Z')
            width += LETTERS_REL_COORDS[c - 'A'][0][0];
        else if (c >= '0' && c <= '9')
            width += DIGITS_REL_COORDS[c - '0'][0][0];

        // Add 1-column gap between letters, not after the last one
        if (i < len - 1)
            width += 1;
    }

    return width;
}

// If a word does not fit on the line (13 pixels wide), calculate the amount of frames needed to scroll the word
// horizontally. First frame is the left side of the word framed up with the left side of the clock. The word scrolls
// left until the last row of the last letter is on the left side of the clock. In that same frame the left side of the
// first letter comes in to frame again. It then scrolls further to the left until reset. This gives 11 frames added to
// the width of the word.
int determine_number_of_frames_word_scroll_animation(const char* word) {
    int word_width = determine_word_display_width(word);

    if (word_width <= 13)
        return 1;
    else
        return (word_width + 11);
}

// Displays a word at a certain row. Row number is the top row at which pixels will be lit. For example, when letters
// are displayed (height 5 pixels) and row 4 is supplied as the argument, pixels will be lit in rows 4-8.
void display_word_at_row(const char* word, int row, int horizontal_offset) {
    int word_width = determine_word_display_width(word);
    int number_of_frames = determine_number_of_frames_word_scroll_animation(word);

    if (number_of_frames == 1) {
        int start_x;
        int start_y = row;

        // Apply horizontal offset
        if (word_width % 2 == 1)
            start_x = 6 - (word_width - 1) / 2 + horizontal_offset;
        else
            start_x = 6 - word_width / 2 + horizontal_offset;

        // Check bounds and correct if necessary
        if (start_x < 0)
            start_x = 0;
        if (start_x + word_width > 13)
            start_x = 13 - word_width;

        int cursor_x = start_x;
        for (int i = 0; word[i] != '\0'; ++i) {
            char c = toupper((unsigned char) word[i]);

            const byte(*coords)[2];
            if (c >= 'A' && c <= 'Z')
                coords = LETTERS_REL_COORDS[c - 'A'];
            else if (c >= '0' && c <= '9')
                coords = DIGITS_REL_COORDS[c - '0'];
            else
                continue;

            byte letter_width = coords[0][0];

            for (int k = 1; k < 14; ++k) {
                if (coords[k][0] == 255)
                    break;
                leds_logical[(start_y + coords[k][1]) * 13 + (cursor_x + coords[k][0])] = CRGB::White;
            }

            cursor_x += letter_width + 1;
        }
    } else {
        int current_frame = (frame_counter / horizontal_scroll_frame_delay) % number_of_frames;
        int start_x = -current_frame;
        int start_y = row;

        int cursor_x = start_x;
        // The animation consists of two instances of the word after each other. The program figures out on the fly
        // which letters can be skipped.
        for (int i = 0; word[i] != '\0'; ++i) {
            char c = toupper((unsigned char) word[i]);

            const byte(*coords)[2];
            if (c >= 'A' && c <= 'Z')
                coords = LETTERS_REL_COORDS[c - 'A'];
            else if (c >= '0' && c <= '9')
                coords = DIGITS_REL_COORDS[c - '0'];
            else
                continue;

            byte letter_width = coords[0][0];

            for (int k = 1; k < 14; ++k) {
                // If the entire letter is outside of the view, skip it
                if ((cursor_x + coords[0][0] <= 0) || (cursor_x > 12))
                    break;
                // If end of letter, break
                if (coords[k][0] == 255)
                    break;
                // If the current coordinate is outside view, continue
                if ((cursor_x + coords[k][0] < 0) || (cursor_x + coords[k][0] > 12))
                    continue;

                leds_logical[(start_y + coords[k][1]) * 13 + (cursor_x + coords[k][0])] = CRGB::White;
            }

            cursor_x += letter_width + 1;
        }

        // Empty space of 11 places (moves 10 because the cursor already includes a 'space' from the last letter)
        cursor_x += 10;

        // Second word instance
        for (int i = 0; word[i] != '\0'; ++i) {
            char c = toupper((unsigned char) word[i]);

            const byte(*coords)[2];
            if (c >= 'A' && c <= 'Z')
                coords = LETTERS_REL_COORDS[c - 'A'];
            else if (c >= '0' && c <= '9')
                coords = DIGITS_REL_COORDS[c - '0'];
            else
                continue;

            byte letter_width = coords[0][0];

            for (int k = 1; k < 14; ++k) {
                // If the entire letter is outside of the view, skip it
                if ((cursor_x + coords[0][0] <= 0) || (cursor_x > 12))
                    break;
                // If end of letter, break
                if (coords[k][0] == 255)
                    break;
                // If the current coordinate is outside view, continue
                if ((cursor_x + coords[k][0] < 0) || (cursor_x + coords[k][0] > 12))
                    continue;

                leds_logical[(start_y + coords[k][1]) * 13 + (cursor_x + coords[k][0])] = CRGB::White;
            }

            cursor_x += letter_width + 1;
        }
    }
}

void LEDController::show_dimmer_set() {
    static int8_t direction = 255 / (ANIMATION_FPS * SELECT_CYCLE_S / 2);
    static int8_t step = direction;

    // Clear all LEDs
    for (int i = 0; i < NUM_LEDS_LOGICAL; ++i) {
        leds_logical[i] = CRGB::Black;
    }

    // Breathing animation for the current text
    if (ANIMATION_STATES[DIMMER_SELECT_BREATHING] < step)
        direction = abs(direction);
    if (ANIMATION_STATES[DIMMER_SELECT_BREATHING] > 255 - step)
        direction = -abs(direction);
    ANIMATION_STATES[DIMMER_SELECT_BREATHING] += direction;

    if (USER_SETTINGS[MANUAL_BRIGHTNESS_SELECT] == 255)
        display_word_at_row("AAN", 4, 0);
    else if (USER_SETTINGS[MANUAL_BRIGHTNESS_SELECT] == 32)
        display_word_at_row("DIM", 4, 0);
    else
        display_word_at_row("UIT", 4, 0);

    // Dim the LEDs (except the minute dots) with the animation state
    for (int i = 0; i < NUM_LEDS_LOGICAL - 5; ++i) {
        leds_logical[i] = blend(CRGB::Black, leds_logical[i], ANIMATION_STATES[DIMMER_SELECT_BREATHING]);
    }

    // Minute dots should slowly fade from right to left as the timeout expires. We devide this time in 10000 bips.
    int bips_elapsed = 10000 * ((long) (millis() - TIMERS[DIMMER_BUTTON_TIMER])) / 15000;
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
            leds_logical[169 + i] = blend(CRGB::Black, CRGB::White,
                                          partial_dot_permyriads * 255 / 2000);  // Partial brightness
        } else {
            leds_logical[169 + i] = CRGB::Black;  // Fully off
        }
    }

    if (FLAGS[FADING_OUT])
        fade_out();
    else if (FLAGS[FADING_IN])
        fade_in();
    else if (FLAGS[CROSSFADING])
        crossfade();
}

void LEDController::show_timer_set() {
    static int8_t direction = 255 / (ANIMATION_FPS * SELECT_CYCLE_S / 2);
    static int8_t step = direction;

    // Clear all LEDs
    for (int i = 0; i < NUM_LEDS_LOGICAL; ++i) {
        leds_logical[i] = CRGB::Black;
    }

    // Breathing animation for the current text
    if (ANIMATION_STATES[TIMER_SELECT_BREATHING] < step)
        direction = abs(direction);
    if (ANIMATION_STATES[TIMER_SELECT_BREATHING] > 255 - step)
        direction = -abs(direction);
    ANIMATION_STATES[TIMER_SELECT_BREATHING] += direction;

    int timer_select_minutes = USER_SETTINGS[TIMER_SELECT_S] / 60;
    int timer_select_seconds = USER_SETTINGS[TIMER_SELECT_S] % 60;

    char buf[4];
    // Display the minutes as 03 or 04 or 14, etc at row 1
    snprintf(buf, sizeof(buf), "%02d", timer_select_minutes);
    display_word_at_row(buf, 1, -2);
    // Display the seconds as 01 or 20 or 44, etc at row 7
    snprintf(buf, sizeof(buf), "%02d", timer_select_seconds);
    display_word_at_row(buf, 7, -2);

    // Dim the LEDs (except the minute dots) with the animation state
    for (int i = 0; i < NUM_LEDS_LOGICAL - 5; ++i) {
        leds_logical[i] = blend(CRGB::Black, leds_logical[i], ANIMATION_STATES[TIMER_SELECT_BREATHING]);
    }

    // Add a static +, - and play at the right side of the clock for the 'controls'
    leds_logical[11] = CRGB::Green;
    leds_logical[23] = CRGB::Green;
    leds_logical[24] = CRGB::Green;
    leds_logical[25] = CRGB::Green;
    leds_logical[37] = CRGB::Green;

    leds_logical[62] = CRGB::Blue;
    leds_logical[75] = CRGB::Blue;
    leds_logical[76] = CRGB::Blue;
    leds_logical[88] = CRGB::Blue;
    leds_logical[89] = CRGB::Blue;
    leds_logical[90] = CRGB::Blue;
    leds_logical[101] = CRGB::Blue;
    leds_logical[102] = CRGB::Blue;
    leds_logical[114] = CRGB::Blue;

    leds_logical[153] = CRGB::Red;
    leds_logical[154] = CRGB::Red;
    leds_logical[155] = CRGB::Red;

    // Minute dots should slowly fade from right to left as the timeout expires. We devide this time in 10000 bips.
    int bips_elapsed =
        10000 * ((long) (millis() - TIMERS[TIMER_BUTTON_TIMER])) / (USER_SETTINGS[TIMER_SELECT_TIMEOUT_S] * 1000);
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
            leds_logical[169 + i] = blend(CRGB::Black, CRGB::White,
                                          partial_dot_permyriads * 255 / 2000);  // Partial brightness
        } else {
            leds_logical[169 + i] = CRGB::Black;  // Fully off
        }
    }

    if (FLAGS[FADING_OUT])
        fade_out();
    else if (FLAGS[FADING_IN])
        fade_in();
    else if (FLAGS[CROSSFADING])
        crossfade();
}

void LEDController::show_timer_running(bool paused) {
    static int8_t direction = 255 / (ANIMATION_FPS * SELECT_CYCLE_S / 2);
    static int8_t step = direction;

    // Clear all LEDs
    for (int i = 0; i < NUM_LEDS_LOGICAL; ++i) {
        leds_logical[i] = CRGB::Black;
    }

    // Breathing animation for the current text if the timer is paused
    if (paused) {
        if (ANIMATION_STATES[TIMER_SELECT_BREATHING] < step)
            direction = abs(direction);
        if (ANIMATION_STATES[TIMER_SELECT_BREATHING] > 255 - step)
            direction = -abs(direction);
        ANIMATION_STATES[TIMER_SELECT_BREATHING] += direction;
    }

    int time_left_s = USER_SETTINGS[TIMER_S] - ((millis() - TIMERS[TIMER_TIMER]) / 1000);
    if (time_left_s < 0)
        time_left_s = 0;

    int timer_select_minutes = time_left_s / 60;
    int timer_select_seconds = time_left_s % 60;

    char buf[4];
    // Display the minutes as 03 or 04 or 25, etc at row 1
    snprintf(buf, sizeof(buf), "%02d", timer_select_minutes);
    display_word_at_row(buf, 1, -2);
    // Display the seconds as 01 or 20 or 44, etc at row 7
    snprintf(buf, sizeof(buf), "%02d", timer_select_seconds);
    display_word_at_row(buf, 7, -2);

    // Dim the LEDs (except the minute dots) with the animation state if the timer is paused
    if (paused) {
        for (int i = 0; i < NUM_LEDS_LOGICAL - 5; ++i) {
            leds_logical[i] = blend(CRGB::Black, leds_logical[i], ANIMATION_STATES[TIMER_SELECT_BREATHING]);
        }
    }

    // Add a static +, pause and stop at the right side of the clock for the 'controls'
    leds_logical[24] = CRGB::Green;
    leds_logical[36] = CRGB::Green;
    leds_logical[37] = CRGB::Green;
    leds_logical[38] = CRGB::Green;
    leds_logical[50] = CRGB::Green;
    if (paused) {
        leds_logical[62] = CRGB::Blue;
        leds_logical[75] = CRGB::Blue;
        leds_logical[76] = CRGB::Blue;
        leds_logical[88] = CRGB::Blue;
        leds_logical[89] = CRGB::Blue;
        leds_logical[90] = CRGB::Blue;
        leds_logical[101] = CRGB::Blue;
        leds_logical[102] = CRGB::Blue;
        leds_logical[114] = CRGB::Blue;
    } else {
        leds_logical[75] = CRGB::Yellow;
        leds_logical[88] = CRGB::Yellow;
        leds_logical[101] = CRGB::Yellow;
        leds_logical[77] = CRGB::Yellow;
        leds_logical[90] = CRGB::Yellow;
        leds_logical[103] = CRGB::Yellow;
    }
    leds_logical[127] = CRGB::Red;
    leds_logical[128] = CRGB::Red;
    leds_logical[129] = CRGB::Red;
    leds_logical[140] = CRGB::Red;
    leds_logical[141] = CRGB::Red;
    leds_logical[142] = CRGB::Red;
    leds_logical[153] = CRGB::Red;
    leds_logical[154] = CRGB::Red;
    leds_logical[155] = CRGB::Red;

    // Minute dots should slowly fade from right to left as the timeout expires. We devide this time in 10000 bips.
    int bips_elapsed = 10000 * (long) (millis() - TIMERS[TIMER_TIMER]) / (1000 * USER_SETTINGS[TIMER_S]);
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
            leds_logical[169 + i] = blend(CRGB::Black, CRGB::White,
                                          partial_dot_permyriads * 255 / 2000);  // Partial brightness
        } else {
            leds_logical[169 + i] = CRGB::Black;  // Fully off
        }
    }

    if (FLAGS[FADING_OUT])
        fade_out();
    else if (FLAGS[FADING_IN])
        fade_in();
    else if (FLAGS[CROSSFADING])
        crossfade();
}

void LEDController::show_timer_finished() {
    // Clear all LEDs
    for (int i = 0; i < NUM_LEDS_LOGICAL; ++i) {
        leds_logical[i] = CRGB::Black;
    }

    // Minute dots should flash intermittently for half a second
    if (millis() % 1000 < 500) {
        display_word_at_row("00", 1, 0);
        display_word_at_row("00", 7, 0);
        for (int i = 0; i < 5; i++)
            leds_logical[169 + i] = CRGB::White;  // Fully on
    } else {
        for (int i = 0; i < 5; i++)
            leds_logical[169 + i] = CRGB::Black;  // Fully off
    }

    if (FLAGS[FADING_OUT])
        fade_out();
    else if (FLAGS[FADING_IN])
        fade_in();
    else if (FLAGS[CROSSFADING])
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
    // Increment frame counter
    frame_counter++;
    reduced_frame_counter = (frame_counter % (USER_SETTINGS[PALETTE_CYCLE_S] * ANIMATION_FPS)) * 256 /
                            (USER_SETTINGS[PALETTE_CYCLE_S] * ANIMATION_FPS);

    // Map logical LED array to physical LEDs
    for (int i = 0; i < NUM_LEDS_PHYSICAL; i++)
        leds_physical[i] = CRGB::Black;
    for (int i = 0; i < NUM_LEDS_LOGICAL; i++)
        leds_physical[L2P[i]] = leds_logical[i];

    // Apply overall brightness based on light sensor values and manual brightness setting
    int fade_amount;
    switch (USER_SETTINGS[MANUAL_BRIGHTNESS]) {
        case 0:
            fade_amount = 255;
            break;
        case 32:
            fade_amount = 223;
            break;
        default:
            // If somehow an invalid value is set for manual brightness, log it and fall back to the default
            // behavior of using the light sensors
            if (USER_SETTINGS[MANUAL_BRIGHTNESS] != 255)
                LOGGER.println("Ongeldige handmatige helderheid (" + String(USER_SETTINGS[MANUAL_BRIGHTNESS]) +
                               "), standaardoptie (lichtsensoren) wordt gebruikt.");

            // Set the overall brightness based on the average light level
            int left_light_average = 0;
            int right_light_average = 0;
            for (int i = 0; i < 10; ++i) {
                left_light_average += LIGHT_SENSOR_VALUES[0][i];
                right_light_average += LIGHT_SENSOR_VALUES[1][i];
            }

            left_light_average /= 10;
            right_light_average /= 10;

            // Low values are light, high values are dark, so take the minimum of both sides
            int max_light_average = min(left_light_average, right_light_average);

            // For fast calculation, assume a range of 2048 to actually apply fading.
            // This 2048 is applied from the value that is assumed 'dark'.
            // There is a cutoff for small light levels to avoid flickering when there is just a little bit of
            // light. Above a certain value (dark value - 2048), just use full brightness.
            if (max_light_average >= 4000) {
                fade_amount = 255;
            } else if (max_light_average >= 3072) {
                fade_amount = 191;
            } else if (max_light_average >= 1536) {
                fade_amount = (max_light_average - 1536) / 8;
            } else {
                fade_amount = 0;
            }

            break;
    }

    for (int i = 0; i < NUM_LEDS_PHYSICAL; ++i)
        if (fade_amount < 255)
            leds_physical[i].fadeToBlackBy(fade_amount);
        else
            leds_physical[i] = CRGB::Black;

    // Apply gamma correction based on average light level, this makes the brightness levels appear more linear to
    // the human eye
    for (int i = 0; i < NUM_LEDS_PHYSICAL; i++)
        leds_physical[i].fadeToBlackBy(
            GAMMA_CORRECTION_PERCENTAGE[max(leds_physical[i].r, max(leds_physical[i].g, leds_physical[i].b))]);

    FastLED.show();
}
