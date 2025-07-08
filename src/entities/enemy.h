#pragma once

#include "entity.h"
#include "../game.h"

void draw_enemy(SDL_Renderer* r, const Entity& e, const Game& g);
void update_enemy(Entity& e);
