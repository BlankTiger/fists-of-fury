#pragma once

#include <SDL3/SDL.h>
#include "entity.h"
#include "../game.h"
#include "../sprite.h"

struct Barrel_Init_Opts { 
    f32           x; 
    f32           y;
    f32           health = 20;
    const Sprite* sprite;
};

Entity barrel_init(Game& g, Barrel_Init_Opts opts);
Update_Result barrel_update(Entity& e, const Game& g);
void barrel_draw(SDL_Renderer* r, const Entity& e, const Game& g);
