#pragma once

#include <SDL3/SDL.h>
#include "entity.h"
#include "../game.h"

struct Barrel_Init_Opts { 
    f32 x; 
    f32 y;
    f32 health = 20;
};

Entity barrel_init(Barrel_Init_Opts opts);
Update_Result barrel_update(Entity& e, Game& g);
void barrel_draw(SDL_Renderer* r, const Entity& e, const Game& g);
