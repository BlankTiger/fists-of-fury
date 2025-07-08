#pragma once

#include "../number_types.h"

#include "../game.h"
#include "entity.h"

enum struct Player_Anim : u32 {
    Standing       = 0,
    Running        = 1,
    Punching_Left  = 2,
    Punching_Right = 3,
    Kicking_Left   = 4,
    Kicking_Right  = 5,
    Kicking_Drop   = 6,
    Got_Hit        = 7,
    Fallover       = 8
};

void start_animation(Entity& e, u32 anim_idx, bool should_loop = false, u64 frame_time = 100);
void update_player(Entity& p, Game& g);
void update_camera(const Entity& p, Game& g);

void draw_player(SDL_Renderer* r, const Entity& p, const Game& g);
