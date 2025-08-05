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
    bool has_knife;
    bool can_spawn_knives;
    bool has_gun;
};

static constexpr Entity_Type dont_collide_with[] = {Entity_Type::Enemy, Entity_Type::Player, Entity_Type::Collectible};
static const Collide_Opts collide_opts = {
    .dont_collide_with = std::span{dont_collide_with},
    .collide_with_walls = false,
};

Entity enemy_init(Game& g, Enemy_Init_Opts opts);
void enemy_draw(SDL_Renderer* r, const Entity& e, Game& g);
Update_Result enemy_update(Entity& e, const Entity& player, Game& g);
