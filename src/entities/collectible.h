#pragma once

#include "entity.h"
#include "../game.h"
#include "../number_types.h"

struct Collectible_Init_Opts {
    Collectible_Type  type;
    Collectible_State state;
    Vec2<f32>         position;
    Direction         dir;
    Entity_Type       done_by;
    bool              instantly_disappear = false;
};

struct Collectible_Drop_Opts {
    bool instantly_disappear = false;
};

Entity        collectible_init(Game& g, Collectible_Init_Opts opts);
Update_Result collectible_update(Entity& e, Game& g);
void          collectible_draw(SDL_Renderer* r, const Entity& e, const Game& g);
void          collectible_throw(Collectible_Type type, Game& g, const Entity& e);
void          collectible_drop(Collectible_Type type, Game& g, const Entity& e, Collectible_Drop_Opts opts = {});


static constexpr Entity_Type knife_dont_collide_with[] = {
    Entity_Type::Barrel,
    Entity_Type::Player,
    Entity_Type::Enemy,
    Entity_Type::Collectible,
};
static const Collide_Opts knife_collide_opts = {
    .dont_collide_with = std::span{knife_dont_collide_with},
    .collide_with_walls = false,
    .reset_position_on_wall_impact = false,
};

static constexpr Entity_Type gun_dont_collide_with[] = {
    Entity_Type::Barrel,
    Entity_Type::Player,
    Entity_Type::Enemy,
    Entity_Type::Collectible,
};
static const Collide_Opts gun_collide_opts = { 
    .dont_collide_with = std::span{gun_dont_collide_with},
    .collide_with_walls = false,
    .reset_position_on_wall_impact = false,
};
