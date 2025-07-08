#pragma once

#include <variant>
#include <SDL3/SDL_main.h>
#include "../number_types.h"

enum struct Direction {
    Up,
    Down,
    Left,
    Right
};

enum struct Entity_Type : u8 {
    Player,
    Enemy,
    Barrel
};

struct Entity {
    int health = 100;
    int damage = 12;
    f32 speed  = 0.03;
    f32 x;
    f32 y;
    u32 idx_anim;
    Direction dir;
    // this should be relative to the player position
    SDL_FRect collision_box_offsets;
    SDL_FRect shadow_offset;

    u32 current_frame      = 0;     // Current frame in the animation
    u64 last_frame_time    = 0;     // When the last frame was shown
    u64 frame_duration_ms  = 100;   // Milliseconds per frame
    bool animation_playing = false;
    bool animation_loop    = true;  // Whether this animation should loop
    u32 default_anim       = 0;     // Animation to return to when current finishes

    Entity_Type type;

    struct Player_Data { };
    struct Enemy_Data  { };

    std::variant<Player_Data, Enemy_Data> extra;
};
