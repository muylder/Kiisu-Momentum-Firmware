// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <stdbool.h>
#include <stdint.h>

#define BARISTA_METHOD_COUNT 30
#define BARISTA_RECIPE_COUNT 64
#define BARISTA_HISTORY_COUNT 12
#define BARISTA_MAX_SECONDS 65535
#define BARISTA_STEP_COUNT 8

typedef enum {
    BaristaEspresso,
    BaristaV60,
    BaristaAeroPress,
    BaristaFrenchPress,
    BaristaCustom,
    BaristaChemex,
    BaristaKalita,
    BaristaClever,
    BaristaMoka,
    BaristaColdBrew,
    BaristaEspressoPreinfusion,
    BaristaHarioSwitch,
    BaristaSiphon,
    BaristaIbrik,
    BaristaPhin,
    BaristaOrea,
    BaristaOrigami,
    BaristaTricolate,
    BaristaPulsar,
    BaristaApril,
    BaristaStagg,
    BaristaPano,
    BaristaMelitta,
    BaristaKoar,
    BaristaChorreador,
    BaristaDelter,
    BaristaGina,
    BaristaBrikka,
    BaristaNapolitana,
    BaristaAeroPressStandard,
} BaristaMethod;

typedef enum {
    BaristaLanguagePortuguese,
    BaristaLanguageEnglish,
} BaristaLanguage;

typedef struct {
    char name[16];
    uint16_t seconds;
    uint16_t water; // Tenths of a gram; cumulative target.
} BaristaStep;

typedef struct {
    char name[32]; // e.g. "Tetsu 4:6"
    char bean[32]; // e.g. "Etiopia"
    uint16_t dose; // Tenths of a gram.
    uint16_t ratio; // Tenths: 20 means 1:2.0.
    uint16_t seconds;
    uint16_t temperature; // Reference only; no sensor.
    uint8_t servings; // Recipe scale, 1..8.
    uint8_t step_count;
    uint8_t method; // Uses BaristaMethod enum
    BaristaStep steps[BARISTA_STEP_COUNT];
    uint8_t grind; // Variable reference setting, 1..100.
} BaristaRecipe;

typedef struct {
    BaristaRecipe recipe;
    uint32_t timestamp;
    uint32_t elapsed;
    uint16_t output; // Tenths of a gram, measured beverage yield.
    uint8_t method;
    uint8_t rating; // 0 = not rated, 1..5 = user rating.
    uint8_t acidity;
    uint8_t body;
    uint8_t sweetness;
    uint8_t bitterness;
    uint8_t finish;
    uint8_t reserved[2];
} BaristaEntry;

typedef struct {
    uint32_t revision;
    BaristaRecipe recipes[BARISTA_RECIPE_COUNT];
    BaristaEntry history[BARISTA_HISTORY_COUNT]; // Newest first.
    uint8_t history_count;
    uint8_t recipe_count;
    uint8_t selected;
    uint8_t language;
    uint8_t grinder_field;
    uint16_t grind_min_clicks;
    uint16_t grind_max_clicks;
} BaristaData;

typedef struct {
    uint32_t ticks;
    uint32_t last_tick;
    bool running;
} BaristaClock;

void barista_defaults(BaristaData* data);
bool barista_data_valid(const BaristaData* data);
uint32_t barista_target(const BaristaRecipe* recipe); // Tenths of a gram, scaled by servings.
uint16_t barista_grind_clicks(const BaristaRecipe* recipe, const BaristaData* data);
uint8_t barista_stage(uint8_t method, const BaristaRecipe* recipe, uint32_t seconds);
uint32_t barista_stage_target(uint8_t method, const BaristaRecipe* recipe, uint8_t stage);
void barista_history_add(BaristaData* data, const BaristaEntry* entry);
void barista_clock_update(BaristaClock* clock, uint32_t now, uint32_t frequency);
