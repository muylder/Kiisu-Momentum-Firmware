// SPDX-License-Identifier: GPL-3.0-or-later
#include "barista_core.h"
#include <string.h>

void barista_defaults(BaristaData* data) {
    memset(data, 0, sizeof(*data));
    data->recipe_count = 30;
    // Editable starting points, not competition recipes or measured values.
    data->recipes[0] = (BaristaRecipe){"Espresso", "", 180, 20, 30, 93, 1, 1, BaristaEspresso, {{"Extrair", 30, 360}}, 24};
    data->recipes[1] = (BaristaRecipe){"V60 - Tetsu 4:6", "", 150, 160, 180, 94, 1, 4, BaristaV60,
        {{"Bloom", 45, 450}, {"Primeiro despejo", 45, 960}, {"Segundo despejo", 45, 1440}, {"Drenar", 45, 2400}}, 24};
    data->recipes[2] = (BaristaRecipe){"AeroPress (Inv)", "", 150, 140, 120, 90, 1, 4, BaristaAeroPress,
        {{"Adicionar agua", 20, 600}, {"Mexer", 20, 1050}, {"Infusao", 60, 2100}, {"Prensar", 20, 2100}}, 24};
    data->recipes[3] = (BaristaRecipe){"Prensa Francesa", "", 200, 150, 240, 94, 1, 3, BaristaFrenchPress,
        {{"Adicionar agua", 30, 1000}, {"Infusao", 180, 3000}, {"Prensar", 30, 3000}}, 24};
    data->recipes[4] = (BaristaRecipe){"Receita Custom", "", 180, 150, 360, 92, 1, 8, BaristaCustom,
        {{"Etapa 1", 45, 338}, {"Etapa 2", 45, 675}, {"Etapa 3", 45, 1013}, {"Etapa 4", 45, 1350},
         {"Etapa 5", 45, 1688}, {"Etapa 6", 45, 2025}, {"Etapa 7", 45, 2363}, {"Finalizar", 45, 2700}}, 24};
    data->recipes[5] = (BaristaRecipe){"Chemex", "", 300, 170, 240, 94, 1, 5, BaristaChemex,
        {{"Bloom", 45, 900}, {"Despejo 1", 45, 2100}, {"Despejo 2", 45, 3300}, {"Despejo 3", 45, 4500}, {"Drenar", 60, 5100}}, 24};
    data->recipes[6] = (BaristaRecipe){"Kalita Wave", "", 200, 160, 210, 94, 1, 4, BaristaKalita,
        {{"Bloom", 40, 600}, {"Despejo 1", 50, 1800}, {"Despejo 2", 60, 3000}, {"Drenar", 60, 3200}}, 24};
    data->recipes[7] = (BaristaRecipe){"Clever", "", 180, 160, 210, 93, 1, 3, BaristaClever,
        {{"Adicionar agua", 30, 1800}, {"Infusao", 150, 2880}, {"Drenar", 30, 2880}}, 24};
    data->recipes[8] = (BaristaRecipe){"Moka", "", 200, 100, 300, 95, 1, 2, BaristaMoka,
        {{"Aquecer", 180, 2000}, {"Finalizar", 120, 2000}}, 24};
    data->recipes[9] = (BaristaRecipe){"Cold Brew", "", 500, 80, 43201, 20, 1, 2, BaristaColdBrew,
        {{"Infusao", 43200, 4000}, {"Filtrar", 1, 4000}}, 24};
    data->recipes[10] = (BaristaRecipe){"Espresso Pre-Inf", "", 180, 20, 35, 93, 1, 2, BaristaEspressoPreinfusion,
        {{"Pre-infusao", 8, 0}, {"Extrair", 27, 360}}, 24};
    data->recipes[11] = (BaristaRecipe){"Hario Switch", "", 200, 150, 180, 95, 1, 3, BaristaHarioSwitch,
        {{"Agua na base", 45, 1000}, {"Infusao", 90, 3000}, {"Drenar", 45, 3000}}, 24};
    data->recipes[12] = (BaristaRecipe){"Sifao", "", 200, 150, 180, 95, 1, 3, BaristaSiphon,
        {{"Aquecer", 60, 0}, {"Misturar", 60, 3000}, {"Descer", 60, 3000}}, 24};
    data->recipes[13] = (BaristaRecipe){"Cezve / Turco", "", 100, 100, 180, 90, 1, 2, BaristaIbrik,
        {{"Ferver 1", 90, 1000}, {"Ferver 2", 90, 1000}}, 24};
    data->recipes[14] = (BaristaRecipe){"Phin (Vietnam)", "", 150, 100, 300, 92, 1, 2, BaristaPhin,
        {{"Bloom", 60, 300}, {"Extrair", 240, 1500}}, 24};
    data->recipes[15] = (BaristaRecipe){"Orea V3", "", 120, 150, 150, 95, 1, 4, BaristaOrea,
        {{"Bloom", 30, 400}, {"Despejo 1", 30, 800}, {"Despejo 2", 30, 1200}, {"Drenar", 60, 1800}}, 24};
    data->recipes[16] = (BaristaRecipe){"Origami", "", 150, 160, 180, 94, 1, 3, BaristaOrigami,
        {{"Bloom", 40, 500}, {"Despejo 1", 50, 1500}, {"Drenar", 90, 2400}}, 24};
    data->recipes[17] = (BaristaRecipe){"Tricolate", "", 150, 220, 360, 98, 1, 4, BaristaTricolate,
        {{"Bloom", 60, 450}, {"Despejo 1", 60, 1500}, {"Despejo 2", 120, 3300}, {"Drenar", 120, 3300}}, 24};
    data->recipes[18] = (BaristaRecipe){"NextLevel Pulsar", "", 200, 160, 300, 95, 1, 4, BaristaPulsar,
        {{"Valv. Fechada", 45, 600}, {"Valv. Aberta", 45, 2000}, {"Despejo 2", 60, 3200}, {"Drenar", 150, 3200}}, 24};
    data->recipes[19] = (BaristaRecipe){"April Brewer", "", 130, 150, 150, 93, 1, 3, BaristaApril,
        {{"Bloom C+Centro", 30, 1000}, {"Centro+Circ", 30, 2000}, {"Drenar", 90, 2000}}, 24};
    data->recipes[20] = (BaristaRecipe){"Fellow Stagg [X]", "", 200, 150, 180, 94, 1, 4, BaristaStagg,
        {{"Bloom", 40, 400}, {"Despejo 1", 40, 1000}, {"Despejo 2", 40, 2000}, {"Despejo 3", 60, 3000}}, 24};
    data->recipes[21] = (BaristaRecipe){"Coador de Pano", "", 200, 100, 240, 96, 1, 3, BaristaPano,
        {{"Pre-infusao", 30, 500}, {"Fio continuo", 150, 2000}, {"Drenar", 60, 2000}}, 24};
    data->recipes[22] = (BaristaRecipe){"Melitta", "", 180, 120, 180, 94, 1, 3, BaristaMelitta,
        {{"Bloom", 30, 400}, {"Despejo lento", 90, 2160}, {"Drenar", 60, 2160}}, 24};
    data->recipes[23] = (BaristaRecipe){"Koar", "", 180, 150, 180, 92, 1, 4, BaristaKoar,
        {{"Bloom", 30, 500}, {"Despejo 1", 30, 1500}, {"Despejo 2", 30, 2700}, {"Drenar", 90, 2700}}, 24};
    data->recipes[24] = (BaristaRecipe){"Chorreador", "", 250, 100, 300, 95, 1, 2, BaristaChorreador,
        {{"Infusao/Gotejo", 180, 2500}, {"Drenar", 120, 2500}}, 24};
    data->recipes[25] = (BaristaRecipe){"Delter Press", "", 120, 160, 180, 94, 1, 4, BaristaDelter,
        {{"Puxar embolo", 10, 0}, {"Injetar (Bloom)", 30, 500}, {"Puxar embolo 2", 20, 0}, {"Injetar", 120, 2000}}, 24};
    data->recipes[26] = (BaristaRecipe){"Gina", "", 200, 150, 180, 95, 1, 3, BaristaGina,
        {{"Valvula Fechada", 60, 1000}, {"Valvula Aberta", 60, 3000}, {"Drenar", 60, 3000}}, 24};
    data->recipes[27] = (BaristaRecipe){"Moka Brikka", "", 200, 80, 240, 96, 1, 2, BaristaBrikka,
        {{"Aquecer pressao", 180, 1600}, {"Extracao rapida", 60, 1600}}, 24};
    data->recipes[28] = (BaristaRecipe){"Napolitana", "", 200, 120, 300, 95, 1, 3, BaristaNapolitana,
        {{"Aquecer base", 240, 0}, {"Virar/Gotejar", 300, 2400}, {"Finalizar", 60, 2400}}, 24};
    data->recipes[29] = (BaristaRecipe){"AeroPress Standard", "", 150, 140, 120, 90, 1, 4, BaristaAeroPressStandard,
        {{"Agua", 20, 2100}, {"Mexer", 10, 2100}, {"Infusao", 60, 2100}, {"Prensar", 30, 2100}}, 24};

    data->language = BaristaLanguagePortuguese;
    data->grind_min_clicks = 1;
    data->grind_max_clicks = 80;
}

static bool barista_recipe_valid(const BaristaRecipe* recipe) {
    return recipe->dose >= 50 && recipe->dose <= 600 && recipe->servings >= 1 && recipe->servings <= 8 &&
           recipe->ratio >= 10 && recipe->ratio <= 220 && recipe->seconds >= 1 &&
           recipe->temperature <= 100 && recipe->method < BARISTA_METHOD_COUNT &&
           recipe->step_count >= 1 && recipe->step_count <= BARISTA_STEP_COUNT && recipe->grind >= 1 && recipe->grind <= 100;
}

bool barista_data_valid(const BaristaData* data) {
    if(data->selected >= data->recipe_count || data->history_count > BARISTA_HISTORY_COUNT ||
       data->language > BaristaLanguageEnglish || data->grind_min_clicks >= data->grind_max_clicks ||
       data->grind_max_clicks > 1000 || data->grinder_field > 1 || data->recipe_count > BARISTA_RECIPE_COUNT) return false;
    for(uint8_t i = 0; i < data->recipe_count; ++i) {
        if(!barista_recipe_valid(&data->recipes[i])) return false;
    }
    for(uint8_t i = 0; i < data->history_count; ++i) {
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
    if(data->history_count < BARISTA_HISTORY_COUNT) ++data->history_count;
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
