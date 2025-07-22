#pragma once

#include <span>
#include <vector>

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

#include "sprite.h"
#include "level_info.h"
#include "entities/entity.h"
#include "vec2.h"
#include "debug_menu.h"

enum struct Update_Result { Remove_Me, None };

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

    Debug_Menu menu;

    TTF_Font* font_tiny_mono;
    TTF_Font* font_press_start_2p;

    Img bg;
    Img entity_shadow;

    static constexpr u32 sprite_player_frames[] = { 4, 8, 4, 3, 6, 6, 1, 3, 3, 1, 1, 1 };
    Sprite sprite_player = {
        .img                     = {},
        .max_frames_in_row_count = 10,
        .frames_in_each_row      = std::span{sprite_player_frames},
    };

    static constexpr u32 sprite_barrel_frames[] = { 1, 1 };
    Sprite sprite_barrel = {
        .img                     = {},
        .max_frames_in_row_count = 1,
        .frames_in_each_row      = std::span{sprite_barrel_frames},
    };

    static constexpr u32 sprite_enemy_frames[] = { 1, 8, 3, 3, 3, 3, 4, 1 };
    Sprite sprite_enemy_goon = {
        .img                     = {},
        .max_frames_in_row_count = 10,
        .frames_in_each_row      = std::span{sprite_enemy_frames},
    };
    Sprite sprite_enemy_punk = {
        .img                     = {},
        .max_frames_in_row_count = 10,
        .frames_in_each_row      = std::span{sprite_enemy_frames},
    };
    Sprite sprite_enemy_thug = {
        .img                     = {},
        .max_frames_in_row_count = 10,
        .frames_in_each_row      = std::span{sprite_enemy_frames},
    };

    static constexpr u32 sprite_enemy_boss_frames[] = { 1, 8, 4, 3, 5, 2, 3, 1, 1, 8 };
    Sprite sprite_enemy_boss = {
        .img                     = {},
        .max_frames_in_row_count = 10,
        .frames_in_each_row      = std::span{sprite_enemy_boss_frames},
    };

    Input_State input;
    Input_State input_prev; // for detecting press -> release

    std::vector<Entity> entities;
    std::vector<u32>    sorted_indices; // for y-sorting when drawing
    std::vector<u32>    removal_queue;  // for removing entities at the end of the frame
    // TODO: in the future make this a unique type, see handles are better pointers
    u32 idx_player;

    Level_Info curr_level_info;
    Camera     camera;
    u64        dt;
};

Vec2<f32> game_get_screen_coords(const Game& g, Vec2<f32> worlds_coords);
Entity game_get_player(const Game& g);
Entity& game_get_player_mutable(Game& g);
