#pragma once
#include "barista_core.h"
#include <storage/storage.h>

bool barista_load_recipes(BaristaData* data, Storage* storage);
bool barista_save_recipes(BaristaData* data, Storage* storage);

