#pragma once

#include <SDL3/SDL_main.h>
#include "entity.h"
#include "../game.h"

void update_barrel(Entity& e);
void draw_barrel(SDL_Renderer* r, const Entity& e, const Game& g);
