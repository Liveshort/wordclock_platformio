#ifndef _LED_FUNCTIONS_H_
#define _LED_FUNCTIONS_H_

#include <Arduino.h>

class LEDController {
   private:
    // Private members can be added here if needed
   public:
    void initialize_led_controller();
    void save_current_state();
    void check_cycle_themes();
    int cycle_theme_now();
    void set_current_theme_to_target();
    void overlay_AP_active(bool ap_active);
    void waiting_for_wifi_breathing_animation();
    void waiting_for_wifi_failed();
    void wifi_connected_blink();
    void waiting_for_time_breathing_animation();
    void time_synced_blink();
    void show_time();
    void show_saying(int saying_index);
    void show_drawing_board();  // Display drawing board LEDs with colors
    void update();
};

// Presave the rows of each LED to reduce the amount of divisions necessary per frame
const byte LEDS_ROW[] = {
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  2,  2,  2,
    2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  4,  4,  4,  4,  4,  4,
    4,  4,  4,  4,  4,  4,  4,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  6,  6,  6,  6,  6,  6,  6,  6,  6,
    6,  6,  6,  6,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
    8,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 11, 11,
    11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 13, 13, 13, 13, 13};

// Define the LED matrix layout conversion from logical to physical
const byte L2P[] = {0,   1,   2,   3,   4,   5,   6,   7,   8,   9,   10,  11,  12,  26,  25,  24,  23,  22,  21,  20,
                    19,  18,  17,  16,  15,  14,  28,  29,  30,  31,  32,  33,  34,  35,  36,  37,  38,  39,  40,  54,
                    53,  52,  51,  50,  49,  48,  47,  46,  45,  44,  43,  42,  56,  57,  58,  59,  60,  61,  62,  63,
                    64,  65,  66,  67,  68,  82,  81,  80,  79,  78,  77,  76,  75,  74,  73,  72,  71,  70,  84,  85,
                    86,  87,  88,  89,  90,  91,  92,  93,  94,  95,  96,  110, 109, 108, 107, 106, 105, 104, 103, 102,
                    101, 100, 99,  98,  112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 138, 137, 136,
                    135, 134, 133, 132, 131, 130, 129, 128, 127, 126, 139, 140, 141, 142, 143, 144, 145, 146, 147, 148,
                    149, 150, 151, 165, 164, 163, 162, 161, 160, 159, 158, 157, 156, 155, 154, 153, 167, 168, 169, 170,
                    171, 172, 173, 174, 175, 176, 177, 178, 179, 185, 184, 183, 182, 181};

enum WORDS {
    HET,
    DIE,
    ER,
    EN,
    NEEM,
    IS,
    DE,
    EEN,
    LATER,
    MEER,
    HOOG,
    TIEN,
    KWART,
    TIJD,
    VIJF,
    IS2,
    OVER,
    VOOR,
    VOORBIJ,
    IETS,
    DAN,
    ANDERS,
    JE,
    HALF,
    NIEUWS,
    VAN,
    ELF,
    KOMT,
    DENKT,
    ACHT,
    TWEE,
    EEN2,
    KOMEN,
    GAAN,
    NEGEN,
    ZEVEN,
    VLIEGT,
    TIEN2,
    VIER,
    DRIE,
    NOG,
    TWAALF,
    VIJF2,
    ZES,
    UUR
};

// Inline definition necessary to avoid multiple definition errors when building
inline const char* WORD_STRINGS[] = {"HET",     "DIE",   "ER",   "EN",     "NEEM", "IS",     "DE",     "EEN",   "LATER",
                                     "MEER",    "HOOG",  "TIEN", "KWART",  "TIJD", "VIJF",   "IS2",    "OVER",  "VOOR",
                                     "VOORBIJ", "IETS",  "DAN",  "ANDERS", "JE",   "HALF",   "NIEUWS", "VAN",   "ELF",
                                     "KOMT",    "DENKT", "ACHT", "TWEE",   "EEN",  "KOMEN",  "GAAN",   "NEGEN", "ZEVEN",
                                     "VLIEGT",  "TIEN",  "VIER", "DRIE",   "NOG",  "TWAALF", "VIJF",   "ZES",   "UUR"};

// Predefined sayings. Some sayings have two variants for more randomness.
// The other sayings are duplicated to keep the random selection uniform.
const byte SAYINGS[][7]{
    {HET, IS, LATER, DAN, JE, DENKT, 255},       // "HET IS LATER DAN JE DENKT"
    {HET, IS, LATER, DAN, JE, DENKT, 255},       // "HET IS LATER DAN JE DENKT"
    {HET, IS, TIJD, 255, 255, 255, 255},         // "HET IS TIJD"
    {HET, IS, HOOG, TIJD, 255, 255, 255},        // "HET IS HOOG TIJD"
    {HET, IS, TIJD, VOOR, IETS, ANDERS, 255},    // "HET IS TIJD VOOR IETS ANDERS"
    {HOOG, TIJD, VOOR, IETS, ANDERS, 255, 255},  // "HOOG TIJD VOOR IETS ANDERS"
    {HET, IS, TIJD, VOOR, IETS, NIEUWS, 255},    // "HET IS TIJD VOOR IETS NIEUWS"
    {HOOG, TIJD, VOOR, IETS, NIEUWS, 255, 255},  // "HOOG TIJD VOOR IETS NIEUWS"
    {NEEM, DE, TIJD, 255, 255, 255, 255},        // "NEEM DE TIJD"
    {NEEM, DE, TIJD, 255, 255, 255, 255},        // "NEEM DE TIJD"
    {ER, IS, MEER, TIJD, DAN, JE, DENKT},        // "ER IS MEER TIJD DAN JE DENKT"
    {ER, IS, MEER, TIJD, DAN, JE, DENKT},        // "ER IS MEER TIJD DAN JE DENKT"
    {ER, IS, EEN, TIJD, VAN, KOMEN, 255},        // "ER IS EEN TIJD VAN KOMEN"
    {ER, IS, EEN, TIJD, VAN, GAAN, 255},         // "ER IS EEN TIJD VAN GAAN"
    {DIE, TIJD, KOMT, NOG, 255, 255, 255},       // "DIE TIJD KOMT NOG"
    {DIE, TIJD, KOMT, NOG, 255, 255, 255},       // "DIE TIJD KOMT NOG"
    {DIE, TIJD, IS2, VOORBIJ, 255, 255, 255},    // "DIE TIJD IS VOORBIJ"
    {DIE, TIJD, IS2, VOORBIJ, 255, 255, 255},    // "DIE TIJD IS VOORBIJ"
    {DE, TIJD, VLIEGT, 255, 255, 255, 255},      // "DE TIJD VLIEGT"
    {DE, TIJD, VLIEGT, 255, 255, 255, 255},      // "DE TIJD VLIEGT"
    {ER, IS, EEN, TIJD, VAN, KOMEN, 255},        // "ER IS EEN TIJD VAN KOMEN"
    {EN, EEN, TIJD, VAN, GAAN, 255, 255}         // "EN EEN TIJD VAN GAAN"
};

// Word to letter location lookup table
const byte W2LL[][7] = {
    {0, 1, 2, 255, 255, 255, 255},        // HET
    {3, 4, 5, 255, 255, 255, 255},        // DIE
    {5, 6, 255, 255, 255, 255, 255},      // ER
    {7, 8, 255, 255, 255, 255, 255},      // EN
    {8, 9, 10, 11, 255, 255, 255},        // NEEM
    {15, 16, 255, 255, 255, 255, 255},    // IS
    {17, 18, 255, 255, 255, 255, 255},    // DE
    {18, 19, 20, 255, 255, 255, 255},     // EEN
    {21, 22, 23, 24, 25, 255, 255},       // LATER
    {26, 27, 28, 29, 255, 255, 255},      // MEER
    {31, 32, 33, 34, 255, 255, 255},      // HOOG
    {35, 36, 37, 38, 255, 255, 255},      // TIEN
    {39, 40, 41, 42, 43, 255, 255},       // KWART
    {43, 44, 45, 46, 255, 255, 255},      // TIJD
    {47, 48, 49, 50, 255, 255, 255},      // VIJF
    {52, 53, 255, 255, 255, 255, 255},    // IS2
    {54, 55, 56, 57, 255, 255, 255},      // OVER
    {58, 59, 60, 61, 255, 255, 255},      // VOOR
    {58, 59, 60, 61, 62, 63, 64},         // VOORBIJ
    {65, 66, 67, 68, 255, 255, 255},      // IETS
    {69, 70, 71, 255, 255, 255, 255},     // DAN
    {70, 71, 72, 73, 74, 75, 255},        // ANDERS
    {76, 77, 255, 255, 255, 255, 255},    // JE
    {78, 79, 80, 81, 255, 255, 255},      // HALF
    {82, 83, 84, 85, 86, 87, 255},        // NIEUWS
    {88, 89, 90, 255, 255, 255, 255},     // VAN
    {92, 93, 94, 255, 255, 255, 255},     // ELF
    {95, 96, 97, 98, 255, 255, 255},      // KOMT
    {99, 100, 101, 102, 103, 255, 255},   // DENKT
    {104, 105, 106, 107, 255, 255, 255},  // ACHT
    {107, 108, 109, 110, 255, 255, 255},  // TWEE
    {109, 110, 111, 255, 255, 255, 255},  // EEN2
    {112, 113, 114, 115, 116, 255, 255},  // KOMEN
    {117, 118, 119, 120, 255, 255, 255},  // GAAN
    {120, 121, 122, 123, 124, 255, 255},  // NEGEN
    {125, 126, 127, 128, 129, 255, 255},  // ZEVEN
    {130, 131, 132, 133, 134, 135, 255},  // VLIEGT
    {135, 136, 137, 138, 255, 255, 255},  // TIEN2
    {139, 140, 141, 142, 255, 255, 255},  // VIER
    {143, 144, 145, 146, 255, 255, 255},  // DRIE
    {147, 148, 149, 255, 255, 255, 255},  // NOG
    {150, 151, 152, 153, 154, 155, 255},  // TWAALF
    {156, 157, 158, 159, 255, 255, 255},  // VIJF2
    {160, 161, 162, 255, 255, 255, 255},  // ZES
    {164, 165, 166, 255, 255, 255, 255}   // UUR
};

const byte GAMMA_CORRECTION_TABLE[256] = {
    0,   0,   0,   1,   1,   1,   2,   2,   2,   3,   3,   3,   3,   4,   4,   4,   5,   5,   6,   6,   7,   7,
    8,   8,   9,   9,   10,  10,  11,  11,  11,  12,  12,  13,  13,  14,  14,  15,  16,  16,  17,  17,  18,  19,
    19,  20,  20,  20,  21,  21,  22,  22,  22,  23,  23,  24,  24,  24,  25,  25,  26,  26,  27,  27,  27,  28,
    28,  29,  29,  30,  30,  31,  32,  32,  33,  33,  34,  34,  35,  35,  36,  37,  37,  38,  39,  39,  40,  40,
    41,  42,  42,  43,  44,  44,  45,  46,  47,  47,  48,  49,  49,  50,  51,  52,  53,  53,  54,  55,  56,  56,
    57,  58,  59,  60,  61,  62,  62,  63,  64,  65,  66,  67,  68,  69,  70,  70,  71,  72,  73,  74,  75,  76,
    77,  78,  79,  80,  81,  82,  83,  84,  85,  86,  87,  88,  89,  91,  92,  93,  94,  95,  96,  97,  98,  99,
    101, 102, 103, 104, 105, 106, 108, 109, 110, 111, 112, 114, 115, 116, 117, 119, 120, 121, 122, 124, 125, 126,
    128, 129, 130, 132, 133, 134, 136, 137, 138, 140, 141, 143, 144, 145, 147, 148, 150, 151, 152, 154, 155, 157,
    158, 160, 161, 163, 164, 166, 167, 169, 170, 172, 173, 175, 177, 178, 180, 181, 183, 184, 186, 188, 189, 191,
    193, 194, 196, 198, 199, 201, 203, 204, 206, 208, 209, 211, 213, 215, 216, 218, 220, 222, 223, 225, 227, 229,
    230, 232, 234, 236, 238, 240, 241, 243, 245, 247, 249, 251, 253, 255};

const byte GAMMA_CORRECTION_PERCENTAGE[256] = {
    255, 255, 255, 170, 191, 204, 170, 182, 191, 170, 178, 185, 191, 176, 182, 187, 175, 180, 170, 174, 165, 170,
    162, 166, 159, 163, 156, 160, 154, 158, 161, 156, 159, 154, 157, 153, 155, 151, 147, 150, 146, 149, 145, 142,
    144, 141, 144, 146, 143, 145, 142, 145, 147, 144, 146, 143, 145, 147, 145, 146, 144, 146, 143, 145, 147, 145,
    146, 144, 146, 144, 145, 143, 141, 143, 141, 142, 140, 142, 140, 142, 140, 138, 139, 138, 136, 138, 136, 137,
    136, 134, 136, 134, 133, 134, 132, 131, 130, 131, 130, 128, 130, 128, 127, 126, 125, 126, 125, 123, 122, 123,
    122, 121, 120, 119, 118, 117, 118, 117, 116, 115, 114, 113, 112, 111, 111, 112, 111, 110, 109, 108, 107, 107,
    106, 105, 104, 103, 103, 102, 101, 100, 100, 99,  98,  98,  97,  94,  94,  93,  93,  92,  91,  91,  90,  90,
    87,  87,  86,  86,  85,  85,  82,  82,  81,  81,  80,  78,  78,  77,  77,  75,  75,  74,  74,  72,  71,  71,
    69,  69,  68,  66,  66,  66,  64,  64,  63,  62,  61,  60,  59,  59,  57,  57,  55,  55,  55,  53,  53,  51,
    51,  49,  49,  48,  47,  46,  46,  44,  44,  43,  42,  41,  40,  39,  38,  38,  36,  36,  35,  34,  33,  32,
    31,  31,  29,  28,  28,  27,  25,  25,  24,  23,  23,  22,  20,  19,  19,  18,  17,  16,  16,  14,  13,  12,
    12,  11,  10,  9,   8,   7,   7,   6,   5,   4,   3,   2,   1,   0};

#endif
