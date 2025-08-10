#pragma once

#include "entity.h"
#include "../game.h"

struct Bullet_Init_Opts {
    Vec2<f32>   pos_creator;
    Vec2<f32>   offsets;
    Direction   dir;
    Entity_Type shot_by;
    f32         length    = SCREEN_WIDTH;
    f32         thickness = 0.6f;
};

Entity bullet_init(Game& g, Bullet_Init_Opts opts);
Update_Result bullet_update(Entity& e, Game& g);
void bullet_draw(SDL_Renderer* r, const Entity& e, const Game& g);
