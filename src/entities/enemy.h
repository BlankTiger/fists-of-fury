#pragma once

#include "entity.h"
#include "../game.h"

void enemy_draw(SDL_Renderer* r, const Entity& e, const Game& g);
Update_Result enemy_update(Entity& e);
