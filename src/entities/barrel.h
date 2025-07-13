#pragma once

#include <SDL3/SDL.h>
#include "entity.h"
#include "../game.h"

Entity barrel_init();
void barrel_update(Entity& e);
void barrel_draw(SDL_Renderer* r, const Entity& e, const Game& g);
