#pragma once

#include "entity.h"
#include "../game.h"

struct Bullet_Init_Opts { };

Entity bullet_init(Game& g, Bullet_Init_Opts opts);
Update_Result bullet_update(Entity& e, Game& g);
void bullet_draw(SDL_Renderer* r, const Entity& e, Game& g);
