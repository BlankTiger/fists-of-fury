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

enum struct Update_Result { None, Remove_Me };

struct Input_State {
    bool left     = false;
    bool right    = false;
    bool up       = false;
    bool down     = false;
    bool attack   = false;
    bool interact = false;
    bool jump     = false;
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

    Sprite sprite_player = {
        .img                     = {},
        .max_frames_in_row_count = 10, // THIS HAS TO CORRESPOND TO THE WIDTH OF THE SPRITE -> SPRITE_WIDTH / FRAME_WIDTH
        .frames_in_each_row      = std::span{sprite_player_frames},
    };
    Sprite sprite_knife_player = {
        .img                     = {},
        .max_frames_in_row_count = 10, // THIS HAS TO CORRESPOND TO THE WIDTH OF THE SPRITE -> SPRITE_WIDTH / FRAME_WIDTH
        .frames_in_each_row      = std::span{sprite_knife_player_frames},
    };
    Sprite sprite_gun_player = {
        .img                     = {},
        .max_frames_in_row_count = 10, // THIS HAS TO CORRESPOND TO THE WIDTH OF THE SPRITE -> SPRITE_WIDTH / FRAME_WIDTH
        .frames_in_each_row      = std::span{sprite_gun_player_frames},
    };

    Sprite sprite_barrel = {
        .img                     = {},
        .max_frames_in_row_count = 1, // THIS HAS TO CORRESPOND TO THE WIDTH OF THE SPRITE -> SPRITE_WIDTH / FRAME_WIDTH
        .frames_in_each_row      = std::span{sprite_barrel_frames},
    };

    Sprite sprite_knife = {
        .img                     = {},
        .max_frames_in_row_count = 1, // THIS HAS TO CORRESPOND TO THE WIDTH OF THE SPRITE -> SPRITE_WIDTH / FRAME_WIDTH
        .frames_in_each_row      = std::span{sprite_knife_frames},
    };

    Sprite sprite_gun = {
        .img                     = {},
        .max_frames_in_row_count = 1, // THIS HAS TO CORRESPOND TO THE WIDTH OF THE SPRITE -> SPRITE_WIDTH / FRAME_WIDTH
        .frames_in_each_row      = std::span{sprite_gun_frames},
    };

    Sprite sprite_knife_enemy = {
        .img                     = {},
        .max_frames_in_row_count = 10, // THIS HAS TO CORRESPOND TO THE WIDTH OF THE SPRITE -> SPRITE_WIDTH / FRAME_WIDTH
        .frames_in_each_row      = std::span{sprite_knife_enemy_frames},
    };
    Sprite sprite_gun_enemy = {
        .img                     = {},
        .max_frames_in_row_count = 10, // THIS HAS TO CORRESPOND TO THE WIDTH OF THE SPRITE -> SPRITE_WIDTH / FRAME_WIDTH
        .frames_in_each_row      = std::span{sprite_gun_enemy_frames},
    };
    Sprite sprite_enemy_goon = {
        .img                     = {},
        .max_frames_in_row_count = 10, // THIS HAS TO CORRESPOND TO THE WIDTH OF THE SPRITE -> SPRITE_WIDTH / FRAME_WIDTH
        .frames_in_each_row      = std::span{sprite_enemy_frames},
    };
    Sprite sprite_enemy_punk = {
        .img                     = {},
        .max_frames_in_row_count = 10, // THIS HAS TO CORRESPOND TO THE WIDTH OF THE SPRITE -> SPRITE_WIDTH / FRAME_WIDTH
        .frames_in_each_row      = std::span{sprite_enemy_frames},
    };
    Sprite sprite_enemy_thug = {
        .img                     = {},
        .max_frames_in_row_count = 10, // THIS HAS TO CORRESPOND TO THE WIDTH OF THE SPRITE -> SPRITE_WIDTH / FRAME_WIDTH
        .frames_in_each_row      = std::span{sprite_enemy_frames},
    };

    Sprite sprite_enemy_boss = {
        .img                     = {},
        .max_frames_in_row_count = 10, // THIS HAS TO CORRESPOND TO THE WIDTH OF THE SPRITE -> SPRITE_WIDTH / FRAME_WIDTH
        .frames_in_each_row      = std::span{sprite_enemy_boss_frames},
    };

    Input_State input;
    Input_State input_prev; // for detecting press -> release

    std::vector<Entity>            entities;
    std::vector<u32>               sorted_indices;      // for y-sorting when drawing
    std::vector<u32>               removal_queue;       // for removing entities at the end of the frame
    std::vector<Prop_Thrown_Info>  props_thrown_queue;  // gets used when collecting props thrown in the current frame and emptied when creating them
    std::vector<Prop_Dropped_Info> props_dropped_queue; // gets used when collecting props dropped in the current frame and emptied when creating them

    // TODO: in the future make this a unique type, see handles are better pointers
    u32 idx_player;

    Level_Info curr_level_info;
    Camera     camera;
    u64        dt; // scaled by settings.time_scale
    u64        dt_real;
    u64        time_ms = 0;
    u32        last_entity_id = 0;
};

Vec2<f32> game_get_screen_coords(const Game& g, Vec2<f32> worlds_coords);
Entity    game_get_player(const Game& g);
Entity&   game_get_player_mutable(Game& g);
Handle    game_generate_entity_handle(Game& g);
Entity*   game_get_mutable_entity_by_handle(Game& g, const Handle& h);
f32       game_get_border_x(const Game& g, Border border);
