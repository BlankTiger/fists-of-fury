#pragma once

#include "settings.h"

struct Debug_Menu {
    bool show   = false;
    Settings& s = settings;
};

struct SDL_Renderer;

void debug_menu_draw(const Debug_Menu& m, SDL_Renderer* r);
void debug_menu_update(Debug_Menu& m);
