// SPDX-License-Identifier: GPL-3.0-or-later
// Project: muylder/Kiisu-Momentum-Firmware. Implementation assisted by AI.
#include "barista_core.h"
#include <furi.h>
#include <furi_hal.h>
#include <gui/gui.h>
#include <gui/elements.h>
#include <input/input.h>
#include <notification/notification_messages.h>
#include <storage/storage.h>
#include <toolbox/saved_struct.h>
#include <datetime/datetime.h>
#include <stdio.h>
#include <stdlib.h>

#define BARISTA_MAGIC 0xBA
#define BARISTA_VERSION 2
#define BARISTA_SLOT_A APP_DATA_PATH("journal_a.bin")
#define BARISTA_SLOT_B APP_DATA_PATH("journal_b.bin")

typedef enum {
    ScreenHome,
    ScreenRecipe,
    ScreenSteps,
    ScreenBrew,
    ScreenAbort,
    ScreenResult,
    ScreenHistory,
    ScreenHelp,
    ScreenGrinder,
} BaristaScreen;

typedef struct {
    Gui* gui;
    NotificationApp* notification;
    Storage* storage;
    ViewPort* viewport;
    FuriMessageQueue* input;
    FuriMutex* mutex;
    BaristaData data;
    BaristaClock clock;
    BaristaEntry result;
    BaristaScreen screen;
    uint32_t frequency;
    uint8_t field;
    uint8_t history_index;
    uint8_t help_page;
    uint8_t stage;
    uint8_t step_index;
    uint8_t step_field;
    uint8_t result_field;
    uint8_t grinder_field;
    uint8_t active_slot;
    bool storage_failed;
    bool save_pending;
    bool dirty;
    bool exit;
    bool light_on;
} BaristaApp;

static const char* const methods_pt[] = {"Espresso", "V60", "AeroPress", "Prensa francesa", "Personalizado", "Chemex", "Kalita Wave", "Clever", "Moka italiana", "Cold brew", "Espresso PI"};
static const char* const methods_en[] = {"Espresso", "V60", "AeroPress", "French press", "Custom", "Chemex", "Kalita Wave", "Clever", "Moka pot", "Cold brew", "Espresso PI"};
static const char* barista_method_name(const BaristaApp* app, uint8_t method) {
    return (app->data.language == BaristaLanguageEnglish ? methods_en : methods_pt)[method];
}

static const char* barista_text(const BaristaApp* app, const char* pt, const char* en) {
    return app->data.language == BaristaLanguageEnglish ? en : pt;
}

static uint8_t* barista_sensory_value(BaristaEntry* entry, uint8_t field) {
    switch(field) {
    case 2: return &entry->acidity;
    case 3: return &entry->body;
    case 4: return &entry->sweetness;
    case 5: return &entry->bitterness;
    default: return &entry->finish;
    }
}

static const char* barista_sensory_name(const BaristaApp* app, uint8_t field) {
    static const char* const pt[] = {"Acidez", "Corpo", "Docura", "Amargor", "Finalizacao"};
    static const char* const en[] = {"Acidity", "Body", "Sweetness", "Bitterness", "Finish"};
    return (app->data.language == BaristaLanguageEnglish ? en : pt)[field - 2];
}

static uint32_t barista_elapsed(const BaristaApp* app) {
    return app->clock.ticks / app->frequency;
}

static void barista_header(Canvas* canvas, const char* title) {
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 2, 10, title);
    canvas_draw_line(canvas, 0, 13, 127, 13);
    canvas_set_font(canvas, FontSecondary);
}

static void barista_footer(Canvas* canvas, const char* text) {
    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str_aligned(canvas, 64, 63, AlignCenter, AlignBottom, text);
}

static void barista_row(Canvas* canvas, uint8_t y, const char* text, bool selected) {
    if(selected) {
        canvas_draw_box(canvas, 0, y - 8, 128, 10);
        canvas_set_color(canvas, ColorWhite);
    }
    canvas_draw_str(canvas, 3, y, text);
    canvas_set_color(canvas, ColorBlack);
}

static void barista_draw(Canvas* canvas, void* context) {
    BaristaApp* app = context;
    furi_mutex_acquire(app->mutex, FuriWaitForever);
    canvas_clear(canvas);
    canvas_set_color(canvas, ColorBlack);
    char text[64];
    uint8_t method = app->data.selected;
    const BaristaRecipe* recipe = &app->data.recipes[method];
    uint32_t total = barista_target(recipe);
    uint32_t elapsed = barista_elapsed(app);

    switch(app->screen) {
    case ScreenHome: {
        barista_header(canvas, app->storage_failed ? "BARISTA  SD!" : "BARISTA");
        uint8_t first = method > 2 ? method - 2 : 0;
        if(first > BARISTA_METHOD_COUNT - 4) first = BARISTA_METHOD_COUNT - 4;
        for(uint8_t i = first; i < first + 4; ++i) {
            barista_row(canvas, 23 + (i - first) * 10, barista_method_name(app, i), i == method);
        }
        barista_footer(canvas, barista_text(app, "< guia  OK receita  > diario", "< guide OK recipe > journal"));
        break;
    }
    case ScreenRecipe: {
        barista_header(canvas, barista_method_name(app, method));
        uint8_t first = app->field > 2 ? app->field - 2 : 0;
        for(uint8_t i = first; i < first + 3; ++i) {
            switch(i) {
            case 0:
                snprintf(text, sizeof(text), "%s %u.%u g", barista_text(app, "Dose       ", "Dose       "), recipe->dose / 10, recipe->dose % 10);
                break;
            case 1:
                snprintf(text, sizeof(text), "%s 1:%u.%u", barista_text(app, "Proporcao  ", "Ratio      "), recipe->ratio / 10, recipe->ratio % 10);
                break;
            case 2:
                snprintf(text, sizeof(text), "%s %u:%02u", barista_text(app, "Tempo      ", "Time       "), recipe->seconds / 60, recipe->seconds % 60);
                break;
            case 3:
                snprintf(text, sizeof(text), "%s %u C", barista_text(app, "Temperatura", "Temperature"), recipe->temperature);
                break;
            case 4:
                snprintf(text, sizeof(text), "%s %u", barista_text(app, "Porcoes    ", "Servings   "), recipe->servings);
                break;
            case 5:
                snprintf(text, sizeof(text), "%s %uc (%u)", barista_text(app, "Moagem", "Grind"),
                         barista_grind_clicks(recipe, &app->data), recipe->grind);
                break;
            case 6:
                snprintf(text, sizeof(text), "Iniciar preparo");
                break;
            case 7:
                snprintf(text, sizeof(text), "Editar etapas");
                break;
            default:
                text[0] = '\0';
                break;
            }
            barista_row(canvas, 24 + (i - first) * 10, text, i == app->field);
        }
        snprintf(text, sizeof(text), "%s: %lu.%lu g", method == BaristaEspresso ? "Bebida alvo" : "Agua total",
                 (unsigned long)(total / 10), (unsigned long)(total % 10));
        canvas_draw_str(canvas, 3, 54, text);
        barista_footer(canvas, app->field == 6 ? "OK iniciar   BACK voltar" :
                       app->field == 7 ? "OK etapas   BACK voltar" : "^v campo  <> valor  OK prox");
        break;
    }
    case ScreenSteps: {
        const BaristaStep* step = &recipe->steps[app->step_index];
        barista_header(canvas, barista_text(app, "Editar etapas", "Edit steps"));
        snprintf(text, sizeof(text), "%u/%u %s", app->step_index + 1, recipe->step_count, step->name);
        canvas_draw_str(canvas, 3, 24, text);
        snprintf(text, sizeof(text), "%s %02u:%02u", barista_text(app, "Duracao", "Duration"), step->seconds / 60, step->seconds % 60);
        barista_row(canvas, 35, text, app->step_field == 0);
        uint32_t scaled_water = (uint32_t)step->water * recipe->servings;
        snprintf(text, sizeof(text), "%s %lu.%lug", barista_text(app, "Agua alvo", "Water target"),
                 (unsigned long)(scaled_water / 10), (unsigned long)(scaled_water % 10));
        barista_row(canvas, 46, text, app->step_field == 1);
        barista_footer(canvas, barista_text(app, "^v campo  <> valor  OK salvar  BACK voltar", "^v field  <> value  OK save  BACK"));
        break;
    }
    case ScreenGrinder:
        barista_header(canvas, barista_text(app, "Config. moedor", "Grinder setup"));
        snprintf(text, sizeof(text), "%s %uc", barista_text(app, "Minimo     ", "Minimum    "), app->data.grind_min_clicks);
        barista_row(canvas, 28, text, app->grinder_field == 0);
        snprintf(text, sizeof(text), "%s %uc", barista_text(app, "Maximo     ", "Maximum    "), app->data.grind_max_clicks);
        barista_row(canvas, 40, text, app->grinder_field == 1);
        canvas_draw_str(canvas, 3, 53, barista_text(app, "Moagens usam este intervalo", "Recipes use this range"));
        barista_footer(canvas, barista_text(app, "^v campo  <> cliques  BACK voltar", "^v field  <> clicks  BACK"));
        break;
    case ScreenBrew:
        barista_header(canvas, app->clock.running ? barista_method_name(app, method) : barista_text(app, "PAUSADO", "PAUSED"));
        snprintf(text, sizeof(text), "%02lu:%02lu", (unsigned long)(elapsed / 60), (unsigned long)(elapsed % 60));
        canvas_set_font(canvas, FontBigNumbers);
        canvas_draw_str_aligned(canvas, 64, 34, AlignCenter, AlignBottom, text);
        canvas_set_font(canvas, FontSecondary);
        const char* step_name = app->stage < recipe->step_count ? recipe->steps[app->stage].name :
            barista_text(app, "Tempo alvo atingido", "Target time reached");
        canvas_draw_str_aligned(canvas, 64, 43, AlignCenter, AlignBottom, step_name);
        total = barista_stage_target(method, recipe, app->stage);
        snprintf(text, sizeof(text), "%s %lu.%lug  %uC", method == BaristaEspresso ? "Saida" : "Ate",
                 (unsigned long)(total / 10), (unsigned long)(total % 10), recipe->temperature);
        canvas_draw_str_aligned(canvas, 64, 52, AlignCenter, AlignBottom, text);
        float progress = recipe->seconds ? (float)elapsed / recipe->seconds : 0.0f;
        elements_progress_bar(canvas, 8, 56, 112, progress > 1.0f ? 1.0f : progress);
        barista_footer(canvas, app->clock.running ? "OK pausa  > fim  BACK cancela" : "OK retoma  > fim  BACK cancela");
        break;
    case ScreenAbort:
        barista_header(canvas, "Descartar preparo?");
        canvas_draw_str(canvas, 3, 27, "Nao salva no diario.");
        canvas_draw_str(canvas, 3, 40, app->clock.running ? "O tempo continua contando." : "O tempo esta pausado.");
        barista_footer(canvas, "OK descartar  BACK retornar");
        break;
    case ScreenResult:
        barista_header(canvas, "Preparo finalizado");
        snprintf(text, sizeof(text), "%s  %lu:%02lu", barista_method_name(app, method),
                 (unsigned long)(app->result.elapsed / 60), (unsigned long)(app->result.elapsed % 60));
        canvas_draw_str(canvas, 3, 25, text);
        snprintf(text, sizeof(text), "Dose %u.%ug x%u / alvo %lu.%lug", recipe->dose / 10, recipe->dose % 10, recipe->servings,
                 (unsigned long)(total / 10), (unsigned long)(total % 10));
        canvas_draw_str(canvas, 3, 37, text);
        uint32_t output = app->result.output;
        uint32_t dose = recipe->dose * recipe->servings;
        uint32_t brew_ratio = dose ? (output * 10 + dose / 2) / dose : 0;
        snprintf(text, sizeof(text), "Rend: %lu.%lug  1:%lu.%lu", (unsigned long)(output / 10), (unsigned long)(output % 10),
                 (unsigned long)(brew_ratio / 10), (unsigned long)(brew_ratio % 10));
        barista_row(canvas, 47, text, app->result_field == 1);
        if(app->result_field == 1) {
            snprintf(text, sizeof(text), "Nota geral: < %u / 5 >", app->result.rating);
        } else if(app->result_field >= 2) {
            snprintf(text, sizeof(text), "%s: < %u / 5 >", barista_sensory_name(app, app->result_field),
                     *barista_sensory_value(&app->result, app->result_field));
        } else {
            snprintf(text, sizeof(text), "Rendimento: %lu.%lug", (unsigned long)(output / 10), (unsigned long)(output % 10));
        }
        barista_row(canvas, 57, text, true);
        barista_footer(canvas, "^v campo  <> valor  OK salvar");
        break;
    case ScreenHistory:
        barista_header(canvas, app->storage_failed ? "Diario: falha no SD" : "Diario de preparo");
        if(app->data.count) {
            const BaristaEntry* entry = &app->data.history[app->history_index];
            snprintf(text, sizeof(text), "%u/%u %s", app->history_index + 1, app->data.count, barista_method_name(app, entry->method));
            canvas_draw_str(canvas, 3, 24, text);
            uint32_t history_dose = entry->recipe.dose * entry->recipe.servings;
            uint32_t history_ratio = history_dose ? (entry->output * 10 + history_dose / 2) / history_dose : 0;
            snprintf(text, sizeof(text), "%u.%ug>%u.%ug 1:%lu.%lu", (unsigned int)(history_dose / 10), (unsigned int)(history_dose % 10),
                     entry->output / 10, entry->output % 10,
                     (unsigned long)(history_ratio / 10), (unsigned long)(history_ratio % 10));
            canvas_draw_str(canvas, 3, 35, text);
            snprintf(text, sizeof(text), "Alvo 1:%u.%u  %lu:%02lu", entry->recipe.ratio / 10, entry->recipe.ratio % 10,
                     (unsigned long)(entry->elapsed / 60), (unsigned long)(entry->elapsed % 60));
            canvas_draw_str(canvas, 3, 46, text);
            DateTime date;
            datetime_timestamp_to_datetime(entry->timestamp, &date);
            snprintf(text, sizeof(text), "%02u/%02u %02u:%02u  Nota %u/5", date.day, date.month, date.hour, date.minute, entry->rating);
            canvas_draw_str(canvas, 3, 56, text);
        } else {
            canvas_draw_str(canvas, 3, 30, "Seu primeiro cafe comeca");
            canvas_draw_str(canvas, 3, 42, "no menu de receitas.");
        }
        barista_footer(canvas, app->storage_failed ? "> tentar salvar  BACK voltar" : "^v registros  OK repetir  BACK");
        break;
    case ScreenHelp:
        barista_header(canvas, "Guia de bancada");
        if(app->help_page == 2) {
            canvas_draw_str(canvas, 2, 25, barista_text(app, "Idioma", "Language"));
            canvas_draw_str(canvas, 2, 38, app->data.language == BaristaLanguageEnglish ? "English" : "Portugues");
            canvas_draw_str(canvas, 2, 50, barista_text(app, "OK alternar", "OK toggle"));
        } else if(app->help_page == 0) {
            canvas_draw_str(canvas, 2, 25, "Espresso: proporcao =");
            canvas_draw_str(canvas, 2, 36, "bebida / dose de cafe.");
            canvas_draw_str(canvas, 2, 47, "Filtrados: agua / dose.");
        } else if(app->help_page == 1) {
            canvas_draw_str(canvas, 2, 25, barista_text(app, "Pesos sao metas.", "Weights are targets."));
            canvas_draw_str(canvas, 2, 36, barista_text(app, "OK: configurar cliques", "OK: set grinder clicks"));
            canvas_draw_str(canvas, 2, 47, barista_text(app, "Use balanca e termometro.", "Use scale and thermometer."));
        } else {
            canvas_draw_str(canvas, 2, 25, "Ajuste uma variavel por vez.");
            canvas_draw_str(canvas, 2, 36, "Prove, anote e compare.");
            canvas_draw_str(canvas, 2, 47, "SD! = dados so na memoria.");
        }
        barista_footer(canvas, barista_text(app, "<> paginas   OK idioma   BACK voltar", "<> pages   OK language   BACK"));
        break;
    }
    furi_mutex_release(app->mutex);
}

static void barista_input(InputEvent* event, void* context) {
    BaristaApp* app = context;
    // Never block the input service on storage or the app thread.
    if(event->type == InputTypeShort || event->type == InputTypeRepeat) {
        furi_message_queue_put(app->input, event, 0);
    }
}

static void barista_load(BaristaApp* app) {
    BaristaData* other = malloc(sizeof(BaristaData));
    bool a = saved_struct_load(BARISTA_SLOT_A, &app->data, sizeof(app->data), BARISTA_MAGIC, BARISTA_VERSION) && barista_data_valid(&app->data);
    bool b = saved_struct_load(BARISTA_SLOT_B, other, sizeof(*other), BARISTA_MAGIC, BARISTA_VERSION) && barista_data_valid(other);
    if(b && (!a || (int32_t)(other->revision - app->data.revision) > 0)) {
        app->data = *other;
        app->active_slot = 1;
    } else if(!a) {
        barista_defaults(&app->data);
    }
    free(other);
    app->storage_failed = storage_sd_status(app->storage) != FSE_OK;
}

static void barista_save(BaristaApp* app) {
    // Alternate slots: a failed write leaves the previously valid snapshot intact.
    BaristaData* snapshot = malloc(sizeof(BaristaData));
    furi_mutex_acquire(app->mutex, FuriWaitForever);
    *snapshot = app->data;
    snapshot->revision++;
    uint8_t next_slot = app->active_slot ^ 1;
    furi_mutex_release(app->mutex);
    bool ok = storage_simply_mkdir(app->storage, APP_DATA_PATH("")) &&
              saved_struct_save(next_slot ? BARISTA_SLOT_B : BARISTA_SLOT_A, snapshot,
                                sizeof(*snapshot), BARISTA_MAGIC, BARISTA_VERSION);
    furi_mutex_acquire(app->mutex, FuriWaitForever);
    app->storage_failed = !ok;
    if(ok) {
        app->data.revision = snapshot->revision;
        app->active_slot = next_slot;
        app->dirty = false;
    }
    furi_mutex_release(app->mutex);
    free(snapshot);
}

static void barista_adjust(BaristaApp* app, int direction) {
    BaristaRecipe* recipe = &app->data.recipes[app->data.selected];
    if(app->field == 4 || app->field == 5) {
        uint8_t* small_value = app->field == 4 ? &recipe->servings : &recipe->grind;
        int maximum = app->field == 4 ? 8 : 100;
        int adjusted = *small_value + direction;
        *small_value = (uint8_t)(adjusted < 1 ? 1 : adjusted > maximum ? maximum : adjusted);
        app->dirty = true;
        return;
    }
    uint16_t* value;
    int low, high, step;
    switch(app->field) {
    case 0: value = &recipe->dose; low = 50; high = 600; step = 1; break;
    case 1: value = &recipe->ratio; low = 10; high = 220; step = 1; break;
    case 2: value = &recipe->seconds; low = 1; high = BARISTA_MAX_SECONDS; step = 5; break;
    case 3: value = &recipe->temperature; low = 0; high = 100; step = 1; break;
    default: return;
    }
    int next = *value + direction * step;
    *value = next < low ? low : next > high ? high : next;
    app->dirty = true;
}

static void barista_adjust_step(BaristaApp* app, int direction) {
    BaristaRecipe* recipe = &app->data.recipes[app->data.selected];
    BaristaStep* step = &recipe->steps[app->step_index];
    uint16_t* value = app->step_field == 0 ? &step->seconds : &step->water;
    int low = app->step_field == 0 ? 5 : 1;
    int high = app->step_field == 0 ? BARISTA_MAX_SECONDS : 6000;
    int step_size = app->step_field == 0 ? 5 : 5;
    int next = *value + direction * step_size;
    *value = next < low ? low : next > high ? high : next;
    if(app->step_field == 0) {
        uint32_t total = 0;
        for(uint8_t i = 0; i < recipe->step_count; ++i) total += recipe->steps[i].seconds;
        recipe->seconds = total > BARISTA_MAX_SECONDS ? BARISTA_MAX_SECONDS : (uint16_t)total;
    }
    app->dirty = true;
}

static void barista_handle(BaristaApp* app, const InputEvent* event) {
    InputKey key = event->key;
    // Repeat is for navigation/adjustment only, never start/save/stop actions.
    if(event->type == InputTypeRepeat &&
       (key == InputKeyOk || key == InputKeyBack || app->screen == ScreenBrew || app->screen == ScreenAbort)) return;
    switch(app->screen) {
    case ScreenHome:
        if(key == InputKeyUp || key == InputKeyDown) {
            app->data.selected = (app->data.selected + (key == InputKeyDown ? 1 : BARISTA_METHOD_COUNT - 1)) % BARISTA_METHOD_COUNT;
            app->dirty = true;
        } else if(key == InputKeyOk) {
            app->field = 0;
            app->screen = ScreenRecipe;
        } else if(key == InputKeyRight) {
            app->history_index = 0;
            app->screen = ScreenHistory;
        } else if(key == InputKeyLeft) {
            app->screen = ScreenHelp;
        } else if(key == InputKeyBack) app->exit = true;
        break;
    case ScreenRecipe:
        if(key == InputKeyUp) app->field = (app->field + 7) % 8;
        else if(key == InputKeyDown) app->field = (app->field + 1) % 8;
        else if(key == InputKeyLeft || key == InputKeyRight) barista_adjust(app, key == InputKeyRight ? 1 : -1);
        else if(key == InputKeyBack) app->screen = ScreenHome;
        else if(key == InputKeyOk) {
            if(app->field == 6) {
                app->clock = (BaristaClock){.last_tick = furi_get_tick(), .running = true};
                app->result = (BaristaEntry){.recipe = app->data.recipes[app->data.selected],
                                           .method = app->data.selected,
                                           .timestamp = furi_hal_rtc_get_timestamp()};
                app->stage = 0;
                app->screen = ScreenBrew;
                app->save_pending = true;
            } else if(app->field == 7) {
                app->step_index = 0;
                app->step_field = 0;
                app->screen = ScreenSteps;
            } else app->field++;
        }
        break;
    case ScreenSteps: {
        BaristaRecipe* recipe = &app->data.recipes[app->data.selected];
        if(key == InputKeyUp) app->step_field = (app->step_field + 1) % 2;
        else if(key == InputKeyDown) app->step_field = (app->step_field + 1) % 2;
        else if(key == InputKeyLeft || key == InputKeyRight) barista_adjust_step(app, key == InputKeyRight ? 1 : -1);
        else if(key == InputKeyBack) app->screen = ScreenRecipe;
        else if(key == InputKeyOk) {
            if(app->step_index + 1 < recipe->step_count) app->step_index++;
            else { app->field = 5; app->screen = ScreenRecipe; }
        }
        break;
    }
    case ScreenBrew:
        if(key == InputKeyOk && barista_elapsed(app) < BARISTA_MAX_SECONDS) app->clock.running = !app->clock.running;
        else if(key == InputKeyRight) {
            app->clock.running = false;
            app->result.elapsed = barista_elapsed(app);
            app->screen = ScreenResult;
        } else if(key == InputKeyBack) app->screen = ScreenAbort;
        break;
    case ScreenAbort:
        if(key == InputKeyBack) app->screen = ScreenBrew;
        else if(key == InputKeyOk) {
            app->clock.running = false;
            app->screen = ScreenHome;
        }
        break;
    case ScreenResult:
        if(key == InputKeyUp) app->result_field = (app->result_field + 6) % 7;
        else if(key == InputKeyDown) app->result_field = (app->result_field + 1) % 7;
        else if(key == InputKeyRight && app->result_field == 0 && app->result.output < 60000) app->result.output += 1;
        else if(key == InputKeyLeft && app->result_field == 0 && app->result.output) app->result.output -= 1;
        else if(key == InputKeyRight && app->result_field == 1 && app->result.rating < 5) app->result.rating++;
        else if(key == InputKeyLeft && app->result_field == 1 && app->result.rating) app->result.rating--;
        else if(key == InputKeyRight && app->result_field >= 2 && *barista_sensory_value(&app->result, app->result_field) < 5) (*barista_sensory_value(&app->result, app->result_field))++;
        else if(key == InputKeyLeft && app->result_field >= 2 && *barista_sensory_value(&app->result, app->result_field)) (*barista_sensory_value(&app->result, app->result_field))--;
        else if(key == InputKeyBack) app->screen = ScreenHome;
        else if(key == InputKeyOk) {
            barista_history_add(&app->data, &app->result);
            app->dirty = true;
            app->save_pending = true;
            app->history_index = 0;
            app->screen = ScreenHistory;
        }
        break;
    case ScreenHistory:
        if(key == InputKeyBack) app->screen = ScreenHome;
        else if(key == InputKeyRight && app->storage_failed) app->save_pending = true;
        else if(key == InputKeyDown && app->history_index + 1 < app->data.count) app->history_index++;
        else if(key == InputKeyUp && app->history_index) app->history_index--;
        else if(key == InputKeyOk && app->data.count) {
            const BaristaEntry* entry = &app->data.history[app->history_index];
            app->data.selected = entry->method;
            app->data.recipes[entry->method] = entry->recipe;
            app->dirty = true;
            app->field = 5;
            app->screen = ScreenRecipe;
        }
        break;
    case ScreenHelp:
        if(key == InputKeyBack) app->screen = ScreenHome;
        else if(key == InputKeyLeft) app->help_page = (app->help_page + 2) % 3;
        else if(key == InputKeyRight) app->help_page = (app->help_page + 1) % 3;
        else if(key == InputKeyOk && app->help_page == 1) {
            app->grinder_field = 0;
            app->screen = ScreenGrinder;
        } else if(key == InputKeyOk && app->help_page == 2) {
            app->data.language = app->data.language == BaristaLanguageEnglish ? BaristaLanguagePortuguese : BaristaLanguageEnglish;
            app->dirty = true;
        }
        break;
    case ScreenGrinder:
        if(key == InputKeyBack) app->screen = ScreenHelp;
        else if(key == InputKeyUp || key == InputKeyDown) app->grinder_field = (app->grinder_field + 1) % 2;
        else if(key == InputKeyLeft || key == InputKeyRight) {
            int delta = key == InputKeyRight ? 1 : -1;
            if(app->grinder_field == 0) {
                int next = app->data.grind_min_clicks + delta;
                if(next < 0) next = 0;
                if(next >= app->data.grind_max_clicks) next = app->data.grind_max_clicks - 1;
                app->data.grind_min_clicks = next;
            } else {
                int next = app->data.grind_max_clicks + delta;
                if(next <= app->data.grind_min_clicks) next = app->data.grind_min_clicks + 1;
                if(next > 1000) next = 1000;
                app->data.grind_max_clicks = next;
            }
            app->dirty = true;
        }
        break;
    }
}

int32_t barista_app(void* context) {
    UNUSED(context);
    BaristaApp* app = calloc(1, sizeof(BaristaApp));
    app->frequency = furi_kernel_get_tick_frequency();
    app->mutex = furi_mutex_alloc(FuriMutexTypeNormal);
    app->input = furi_message_queue_alloc(16, sizeof(InputEvent));
    app->gui = furi_record_open(RECORD_GUI);
    app->notification = furi_record_open(RECORD_NOTIFICATION);
    app->storage = furi_record_open(RECORD_STORAGE);
    barista_load(app);
    app->viewport = view_port_alloc();
    view_port_draw_callback_set(app->viewport, barista_draw, app);
    view_port_input_callback_set(app->viewport, barista_input, app);
    gui_add_view_port(app->gui, app->viewport, GuiLayerFullscreen);

    while(!app->exit) {
        InputEvent event;
        bool got_input = furi_message_queue_get(app->input, &event, furi_ms_to_ticks(100)) == FuriStatusOk;
        furi_mutex_acquire(app->mutex, FuriWaitForever);
        barista_clock_update(&app->clock, furi_get_tick(), app->frequency);
        bool brewing = app->screen == ScreenBrew || app->screen == ScreenAbort;
        uint8_t stage = barista_stage(app->data.selected, &app->data.recipes[app->data.selected], barista_elapsed(app));
        bool alert = brewing && stage != app->stage;
        if(brewing) app->stage = stage;
        if(got_input) barista_handle(app, &event);
        brewing = app->screen == ScreenBrew || app->screen == ScreenAbort;
        bool save = app->save_pending;
        app->save_pending = false;
        furi_mutex_release(app->mutex);

        if(alert) notification_message(app->notification, &sequence_single_vibro);
        if(brewing != app->light_on) {
            notification_message(app->notification, brewing ? &sequence_display_backlight_enforce_on : &sequence_display_backlight_enforce_auto);
            app->light_on = brewing;
        }
        if(save) barista_save(app);
        view_port_update(app->viewport);
    }

    if(app->dirty) barista_save(app);
    view_port_enabled_set(app->viewport, false);
    gui_remove_view_port(app->gui, app->viewport);
    view_port_free(app->viewport);
    notification_message(app->notification, &sequence_display_backlight_enforce_auto);
    furi_record_close(RECORD_NOTIFICATION);
    furi_record_close(RECORD_STORAGE);
    furi_record_close(RECORD_GUI);
    furi_message_queue_free(app->input);
    furi_mutex_free(app->mutex);
    free(app);
    return 0;
}
