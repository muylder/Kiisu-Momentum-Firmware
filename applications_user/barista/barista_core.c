// SPDX-License-Identifier: GPL-3.0-or-later
#include "barista_core.h"
#include <string.h>

void barista_defaults(BaristaData* data) {
    memset(data, 0, sizeof(*data));
    // Editable starting points, not competition recipes or measured values.
    data->recipes[BaristaEspresso] = (BaristaRecipe){180, 20, 30, 93, 1, 1, 0, {{"Extrair", 30, 360}}, 24};
    data->recipes[BaristaV60] = (BaristaRecipe){150, 160, 180, 94, 1, 4, 0,
        {{"Bloom", 45, 450}, {"Primeiro despejo", 45, 960}, {"Segundo despejo", 45, 1440}, {"Drenar", 45, 2400}}, 24};
    data->recipes[BaristaAeroPress] = (BaristaRecipe){150, 140, 120, 90, 1, 4, 0,
        {{"Adicionar agua", 20, 600}, {"Mexer", 20, 1050}, {"Infusao", 60, 2100}, {"Prensar", 20, 2100}}, 24};
    data->recipes[BaristaFrenchPress] = (BaristaRecipe){200, 150, 240, 94, 1, 3, 0,
        {{"Adicionar agua", 30, 1000}, {"Infusao", 180, 3000}, {"Prensar", 30, 3000}}, 24};
    data->recipes[BaristaCustom] = (BaristaRecipe){180, 150, 360, 92, 1, 8, 0,
        {{"Etapa 1", 45, 338}, {"Etapa 2", 45, 675}, {"Etapa 3", 45, 1013}, {"Etapa 4", 45, 1350},
         {"Etapa 5", 45, 1688}, {"Etapa 6", 45, 2025}, {"Etapa 7", 45, 2363}, {"Finalizar", 45, 2700}}, 24};
    data->recipes[BaristaChemex] = (BaristaRecipe){300, 170, 240, 94, 1, 5, 0,
        {{"Bloom", 45, 900}, {"Despejo 1", 45, 2100}, {"Despejo 2", 45, 3300}, {"Despejo 3", 45, 4500}, {"Drenar", 60, 5100}}, 24};
    data->recipes[BaristaKalita] = (BaristaRecipe){200, 160, 210, 94, 1, 4, 0,
        {{"Bloom", 40, 600}, {"Despejo 1", 50, 1800}, {"Despejo 2", 60, 3000}, {"Drenar", 60, 3200}}, 24};
    data->recipes[BaristaClever] = (BaristaRecipe){180, 160, 210, 93, 1, 3, 0,
        {{"Adicionar agua", 30, 1800}, {"Infusao", 150, 2880}, {"Drenar", 30, 2880}}, 24};
    data->recipes[BaristaMoka] = (BaristaRecipe){200, 100, 300, 95, 1, 2, 0,
        {{"Aquecer", 180, 2000}, {"Finalizar", 120, 2000}}, 24};
    data->recipes[BaristaColdBrew] = (BaristaRecipe){500, 80, 43201, 20, 1, 2, 0,
        {{"Infusao", 43200, 4000}, {"Filtrar", 1, 4000}}, 24};
    data->recipes[BaristaEspressoPreinfusion] = (BaristaRecipe){180, 20, 35, 93, 1, 2, 0,
        {{"Pre-infusao", 8, 0}, {"Extrair", 27, 360}}, 24};
    data->language = BaristaLanguagePortuguese;
    data->grind_min_clicks = 1;
    data->grind_max_clicks = 80;
    for(uint8_t i = 0; i < BARISTA_METHOD_COUNT; ++i) data->recipes[i].grind = 24;
}

static bool barista_recipe_valid(const BaristaRecipe* recipe) {
    return recipe->dose >= 50 && recipe->dose <= 600 && recipe->servings >= 1 && recipe->servings <= 8 &&
           recipe->ratio >= 10 && recipe->ratio <= 220 && recipe->seconds >= 1 &&
           recipe->temperature <= 100 &&
           recipe->step_count >= 1 && recipe->step_count <= BARISTA_STEP_COUNT && recipe->grind >= 1 && recipe->grind <= 100;
}

bool barista_data_valid(const BaristaData* data) {
    if(data->selected >= BARISTA_METHOD_COUNT || data->count > BARISTA_HISTORY_COUNT ||
       data->language > BaristaLanguageEnglish || data->grind_min_clicks >= data->grind_max_clicks ||
       data->grind_max_clicks > 1000 || data->grinder_field > 1) return false;
    for(uint8_t i = 0; i < BARISTA_METHOD_COUNT; ++i) {
        if(!barista_recipe_valid(&data->recipes[i])) return false;
    }
    for(uint8_t i = 0; i < data->count; ++i) {
        const BaristaEntry* entry = &data->history[i];
        if(entry->method >= BARISTA_METHOD_COUNT || entry->rating > 5 || entry->acidity > 5 || entry->body > 5 ||
           entry->sweetness > 5 || entry->bitterness > 5 || entry->finish > 5 || entry->output > 60000 ||
           entry->elapsed > BARISTA_MAX_SECONDS || !barista_recipe_valid(&entry->recipe)) {
            return false;
        }
    }
    return true;
}

uint32_t barista_target(const BaristaRecipe* recipe) {
    return ((uint32_t)recipe->dose * recipe->ratio * recipe->servings + 5) / 10;
}

uint16_t barista_grind_clicks(const BaristaRecipe* recipe, const BaristaData* data) {
    uint32_t span = data->grind_max_clicks - data->grind_min_clicks;
    return data->grind_min_clicks + (uint16_t)(((uint32_t)(recipe->grind - 1) * span + 49) / 99);
}

uint8_t barista_stage(uint8_t method, const BaristaRecipe* recipe, uint32_t seconds) {
    (void)method;
    uint32_t elapsed = 0;
    for(uint8_t i = 0; i < recipe->step_count; ++i) {
        elapsed += recipe->steps[i].seconds;
        if(seconds < elapsed) return i;
    }
    return recipe->step_count;
}

uint32_t barista_stage_target(uint8_t method, const BaristaRecipe* recipe, uint8_t stage) {
    (void)method;
    if(stage < recipe->step_count) return recipe->steps[stage].water * recipe->servings;
    return barista_target(recipe);
}

void barista_history_add(BaristaData* data, const BaristaEntry* entry) {
    memmove(&data->history[1], &data->history[0],
            sizeof(BaristaEntry) * (BARISTA_HISTORY_COUNT - 1));
    data->history[0] = *entry;
    if(data->count < BARISTA_HISTORY_COUNT) ++data->count;
}

void barista_clock_update(BaristaClock* clock, uint32_t now, uint32_t frequency) {
    uint32_t delta = now - clock->last_tick; // Unsigned subtraction handles tick rollover.
    clock->last_tick = now;
    if(!clock->running || !frequency) return;
    uint32_t limit = BARISTA_MAX_SECONDS * frequency;
    if(clock->ticks >= limit || delta >= limit - clock->ticks) {
        clock->ticks = limit;
        clock->running = false;
    } else {
        clock->ticks += delta;
    }
}
