#pragma once

#include "settings.h"

struct Debug_Menu {
    int width_font   = 12;
    int width_border = 2;
    int height_box   = 15;
    int width_box    = 15;
    bool show        = false;
    Settings& s      = settings;
};

struct SDL_Renderer;

void debug_menu_draw(const Debug_Menu& m, SDL_Renderer* r);
void debug_menu_update(Debug_Menu& m);
