#pragma once

#include "entity.h"
#include "../game.h"
#include "../number_types.h"

struct Gun_Init_Opts { };

Entity        gun_init(Game& g, Gun_Init_Opts opts);
Update_Result gun_update(Entity& e, Game& g);
void          gun_draw(SDL_Renderer* r, const Entity& e, const Game& g);

static constexpr Entity_Type gun_dont_collide_with[] = {
    Entity_Type::Barrel,
    Entity_Type::Player,
    Entity_Type::Enemy
};
static const Collide_Opts gun_collide_opts = { 
    .dont_collide_with = std::span{gun_dont_collide_with},
    .collide_with_walls = false,
    .reset_position_on_wall_impact = false,
};
