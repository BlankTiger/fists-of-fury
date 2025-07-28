#pragma once

#include "entity.h"
#include "../game.h"
#include "../number_types.h"

struct Knife_Init_Opts {
    Vec2<f32>   position;
    Direction   dir;
    Knife_State state;
    Entity_Type done_by;
    bool        instantly_disappear = false;
};

static constexpr Entity_Type knife_dont_collide_with[] = {
    Entity_Type::Barrel,
    Entity_Type::Player,
    Entity_Type::Enemy
};
static const Collide_Opts knife_collide_opts = { 
    .dont_collide_with = std::span{knife_dont_collide_with},
    .collide_with_walls = false,
    .reset_position_on_wall_impact = false,
};

Entity knife_init(Game& g, Knife_Init_Opts opts);
Update_Result knife_update(Entity& e, Game& g);
void knife_draw(SDL_Renderer* r, const Entity& e, const Game& g);
void knife_throw(Game& g, const Entity& e);

struct Knife_Drop_Opts {
    bool instantly_disappear = false;
};

void knife_drop(Game& g, const Entity& e, Knife_Drop_Opts opts = {});
