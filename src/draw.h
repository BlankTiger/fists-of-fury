#pragma once

#include <SDL3/SDL_main.h>
#include "game.h"

void draw_collision_box(SDL_Renderer* r, const SDL_FRect& box);
void draw_level(SDL_Renderer* r, const Game& g);
