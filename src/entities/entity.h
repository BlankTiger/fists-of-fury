#pragma once

#include <variant>
#include <SDL3/SDL.h>
#include "../number_types.h"
#include "../vec2.h"

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

enum struct Player_State : u32 {
    Standing,
    Running,
    Punching,
    Kicking,
    Kicking_Drop,
    Got_Hit,
    Dying,
    Takeoff,
    Jumping,
    Landing
};

static_assert((u32)Player_State::Standing == 0);
static_assert((u32)Player_State::Running  == 1);

// this has to be synced with the player sprite
// each enum value is the next row in the sprite
enum struct Player_Anim : u32 {
    Standing,
    Running,
    Punching_Left,
    Punching_Right,
    Kicking_Left,
    Kicking_Right,
    Kicking_Drop,
    Got_Hit,
    Dying,
    Takeoff,
    Jumping,
    Landing
};

struct Entity {
    int health = 100;
    int damage = 12;
    f32 speed  = 0.03;
    f32 x;
    f32 y;
    f32 z;
    f32 z_vel;

    f32 sprite_frame_w;
    f32 sprite_frame_h;
    u32 idx_anim;
    Direction dir;
    // this should be relative to the player position (which is always
    // considered to be where the center of the bottom border of the drawn
    // sprite is)
    SDL_FRect collision_box_offsets;
    // same thing goes for shadow_offset
    SDL_FRect shadow_offsets;

    u32 current_frame      = 0;     // Current frame in the animation
    u64 last_frame_time    = 0;     // When the last frame was shown
    u64 frame_duration_ms  = 100;   // Milliseconds per frame
    bool animation_playing = false;
    bool animation_loop    = true;  // Whether this animation should loop

    // extra data unique to an Entity_Type
    Entity_Type type;
    union {
        struct {
            Player_State state;
            Player_Anim  default_anim;
        } extra_player;

        struct {} extra_enemy;
    };
};

void start_animation(Entity& e, u32 anim_idx, bool should_loop, u64 frame_time);
// gives coordinates such that when they are used to draw the center point of the bottom border
// of the sprite is at the entity x, y
Vec2<f32> entity_offset_to_bottom_center(const Entity& e);

SDL_FRect entity_get_world_collision_box(const Entity& e);
