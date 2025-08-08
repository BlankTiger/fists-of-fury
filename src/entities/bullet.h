#pragma once

#include "entity.h"
#include "../game.h"

struct Bullet_Init_Opts {
    Vec2<f32> pos_start;
    f32 length;
    f32 thickness = 2.0f;
};

Entity bullet_init(Game& g, Bullet_Init_Opts opts);
Update_Result bullet_update(Entity& e, Game& g);
void bullet_draw(SDL_Renderer* r, const Entity& e, Game& g);
