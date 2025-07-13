#pragma once

#include <span>
#include <vector>

#include <SDL3/SDL.h>

#include "sprite.h"
#include "level_info.h"
#include "entities/entity.h"
#include "vec2.h"

enum struct Kick_State { Left, Right };

struct Input_State {
    bool left  = false;
    bool right = false;
    bool up    = false;
    bool down  = false;
    bool punch = false;
    bool kick  = false;
    bool jump  = false;

    Kick_State last_kick     = Kick_State::Left;
    bool last_punch_was_left = true;
};

using Camera = SDL_FRect;

struct Game {
    SDL_Window*   window;
    SDL_Renderer* renderer;

    Img bg;
    Img entity_shadow;

    static constexpr u32 sprite_player_frames[] = { 4, 8, 4, 3, 6, 6, 5, 3, 3, 1, 1, 1 };
    Sprite sprite_player = {
        .img                     = {},
        .max_frames_in_row_count = 10,
        .frames_in_each_row      = std::span{sprite_player_frames},
    };

    static constexpr u32 sprite_barrel_frames[] = { 2 };
    Sprite sprite_barrel = {
        .img = {},
        .max_frames_in_row_count = 2,
        .frames_in_each_row      = std::span{sprite_barrel_frames},
    };

    Input_State input;
    Input_State input_prev; // for detecting press -> release

    std::vector<Entity> entities;
    std::vector<u32>    sorted_indices; // for y-sorting when drawing
    u32 idx_player;

    Level_Info curr_level_info;
    Camera     camera;
    u64        dt;
};

Vec2<f32> game_get_screen_coords(const Game& g, Vec2<f32> worlds_coords);
