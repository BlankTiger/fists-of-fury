#pragma once

#include "entity.h"
#include "../game.h"

struct Enemy_Init_Opts { 
    Enemy_Type type;
    f32 health;
    f32 damage;
    f32 speed;
    f32 x;
    f32 y;
};

Entity enemy_init(const Game& g, Enemy_Init_Opts opts);
void enemy_draw(SDL_Renderer* r, const Entity& e, const Game& g);
Update_Result enemy_update(Entity& e);
