#pragma once

#include "../number_types.h"

#include "../game.h"
#include "entity.h"

enum struct Player_State : u32 {
    Standing,
    Running,
    Punching_Left,
    Punching_Right,
    Kicking_Left,
    Kicking_Right,
    Kicking_Drop,
    Got_Hit,
    Fallover,
    Takeoff,
    Jump,
};

static_assert((u32)Player_State::Standing == 0);
static_assert((u32)Player_State::Running  == 1);

void start_animation(Entity& e, u32 anim_idx, bool should_loop = false, u64 frame_time = 100);
void player_update(Entity& p, Game& g);
void player_draw(SDL_Renderer* r, const Entity& p, const Game& g);
