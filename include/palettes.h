#ifndef _PALETTES_H_
#define _PALETTES_H_

#include <Arduino.h>
#include <FastLED.h>

extern const TProgmemRGBGradientPalette_byte White_p[];
extern const TProgmemRGBGradientPalette_byte CottonCandy_p[];
extern const TProgmemRGBGradientPalette_byte GoldenSun_p[];
extern const TProgmemRGBGradientPalette_byte SunnyDay_p[];
extern const TProgmemRGBGradientPalette_byte BlueSky_p[];
extern const TProgmemRGBGradientPalette_byte BloodyRed_p[];
extern const TProgmemRGBGradientPalette_byte ForestGreen_p[];
extern const TProgmemRGBGradientPalette_byte Peach_p[];
extern const TProgmemRGBGradientPalette_byte TropicalSea_p[];

struct PaletteInfo {
    const char* name;
    CRGBPalette16 pal;
};

// single-definition declarations (defined in src/palettes.cpp)
extern const PaletteInfo PALETTES[];
extern const uint8_t PALETTE_COUNT;

// function declaration
String palette_json_for(uint8_t idx);

#endif
