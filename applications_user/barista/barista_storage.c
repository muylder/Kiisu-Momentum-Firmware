#include "barista_storage.h"
#include <flipper_format/flipper_format.h>
#include <furi.h>

#define BARISTA_RECIPES_DIR EXT_PATH("barista/recipes")
#define RECIPE_FILETYPE "Barista Recipe"
#define RECIPE_VERSION 1

static bool barista_save_recipe_file(Storage* storage, const BaristaRecipe* recipe, const char* filename) {
    char path[300];
    snprintf(path, sizeof(path), "%s/%s", BARISTA_RECIPES_DIR, filename);
    
    FlipperFormat* file = flipper_format_file_alloc(storage);
    bool res = false;
    do {
        if(!flipper_format_file_open_always(file, path)) break;
        if(!flipper_format_write_header_cstr(file, RECIPE_FILETYPE, RECIPE_VERSION)) break;
        if(!flipper_format_write_string_cstr(file, "Name", recipe->name[0] ? recipe->name : "Recipe")) break;
        if(!flipper_format_write_string_cstr(file, "Bean", recipe->bean)) break;
        uint32_t val32;
        val32 = recipe->method; if(!flipper_format_write_uint32(file, "Method", &val32, 1)) break;
        val32 = recipe->dose; if(!flipper_format_write_uint32(file, "Dose", &val32, 1)) break;
        val32 = recipe->ratio; if(!flipper_format_write_uint32(file, "Ratio", &val32, 1)) break;
        val32 = recipe->temperature; if(!flipper_format_write_uint32(file, "Temperature", &val32, 1)) break;
        val32 = recipe->grind; if(!flipper_format_write_uint32(file, "Grind", &val32, 1)) break;
        val32 = recipe->step_count; if(!flipper_format_write_uint32(file, "StepCount", &val32, 1)) break;
        
        for(uint8_t i = 0; i < recipe->step_count; i++) {
            char key[32];
            snprintf(key, sizeof(key), "Step%d_Name", i);
            if(!flipper_format_write_string_cstr(file, key, recipe->steps[i].name)) break;
            snprintf(key, sizeof(key), "Step%d_Secs", i);
            val32 = recipe->steps[i].seconds;
            if(!flipper_format_write_uint32(file, key, &val32, 1)) break;
            snprintf(key, sizeof(key), "Step%d_Water", i);
            val32 = recipe->steps[i].water;
            if(!flipper_format_write_uint32(file, key, &val32, 1)) break;
        }
        res = true;
    } while(0);
    flipper_format_free(file);
    return res;
}

bool barista_save_recipes(BaristaData* data, Storage* storage) {
    storage_simply_mkdir(storage, EXT_PATH("barista"));
    storage_simply_mkdir(storage, BARISTA_RECIPES_DIR);
    for(uint8_t i = 0; i < data->recipe_count; i++) {
        char filename[64];
        snprintf(filename, sizeof(filename), "recipe_%02d.txt", i);
        barista_save_recipe_file(storage, &data->recipes[i], filename);
    }
    return true;
}

static bool barista_load_recipe_file(Storage* storage, BaristaRecipe* recipe, const char* filename) {
    char path[300];
    snprintf(path, sizeof(path), "%s/%s", BARISTA_RECIPES_DIR, filename);
    
    FlipperFormat* file = flipper_format_file_alloc(storage);
    bool res = false;
    do {
        if(!flipper_format_file_open_existing(file, path)) break;
        
        uint32_t version;
        FuriString* filetype = furi_string_alloc();
        bool header_ok = flipper_format_read_header(file, filetype, &version);
        if(!header_ok || furi_string_cmp_str(filetype, RECIPE_FILETYPE) != 0 || version != RECIPE_VERSION) {
            furi_string_free(filetype);
            break;
        }
        furi_string_free(filetype);
        
        FuriString* str_val = furi_string_alloc();
        if(flipper_format_read_string(file, "Name", str_val)) {
            snprintf(recipe->name, sizeof(recipe->name), "%s", furi_string_get_cstr(str_val));
        }
        if(flipper_format_read_string(file, "Bean", str_val)) {
            snprintf(recipe->bean, sizeof(recipe->bean), "%s", furi_string_get_cstr(str_val));
        }
        furi_string_free(str_val);
        
        uint32_t val32;
        if(flipper_format_read_uint32(file, "Method", &val32, 1)) recipe->method = (uint8_t)val32;
        if(flipper_format_read_uint32(file, "Dose", &val32, 1)) recipe->dose = (uint16_t)val32;
        if(flipper_format_read_uint32(file, "Ratio", &val32, 1)) recipe->ratio = (uint16_t)val32;
        if(flipper_format_read_uint32(file, "Temperature", &val32, 1)) recipe->temperature = (uint16_t)val32;
        if(flipper_format_read_uint32(file, "Grind", &val32, 1)) recipe->grind = (uint8_t)val32;
        if(flipper_format_read_uint32(file, "StepCount", &val32, 1)) recipe->step_count = (uint8_t)val32;
        
        recipe->servings = 1;
        
        for(uint8_t i = 0; i < recipe->step_count && i < BARISTA_STEP_COUNT; i++) {
            char key[32];
            snprintf(key, sizeof(key), "Step%d_Name", i);
            FuriString* step_name = furi_string_alloc();
            if(flipper_format_read_string(file, key, step_name)) {
                snprintf(recipe->steps[i].name, sizeof(recipe->steps[i].name), "%s", furi_string_get_cstr(step_name));
            }
            furi_string_free(step_name);
            
            snprintf(key, sizeof(key), "Step%d_Secs", i);
            if(flipper_format_read_uint32(file, key, &val32, 1)) recipe->steps[i].seconds = (uint16_t)val32;
            snprintf(key, sizeof(key), "Step%d_Water", i);
            if(flipper_format_read_uint32(file, key, &val32, 1)) recipe->steps[i].water = (uint16_t)val32;
        }
        res = true;
    } while(0);
    flipper_format_free(file);
    return res;
}

bool barista_load_recipes(BaristaData* data, Storage* storage) {
    File* dir = storage_file_alloc(storage);
    bool res = false;
    data->recipe_count = 0;
    if(storage_dir_open(dir, BARISTA_RECIPES_DIR)) {
        FileInfo info;
        char name[256];
        while(storage_dir_read(dir, &info, name, sizeof(name)) && data->recipe_count < BARISTA_RECIPE_COUNT) {
            if(!file_info_is_dir(&info) && strstr(name, ".txt")) {
                if(barista_load_recipe_file(storage, &data->recipes[data->recipe_count], name)) {
                    data->recipe_count++;
                }
            }
        }
        res = (data->recipe_count > 0);
    }
    storage_dir_close(dir);
    storage_file_free(dir);
    return res;
}

