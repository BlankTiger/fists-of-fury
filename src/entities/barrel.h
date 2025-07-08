#pragma once

#include <SDL3/SDL_main.h>
#include "entity.h"
#include "../game.h"

void barrel_update(Entity& e);
void barrel_draw(SDL_Renderer* r, const Entity& e, const Game& g);
