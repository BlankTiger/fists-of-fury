#pragma once

#include <vector>
#include <SDL3/SDL.h>
#include "../number_types.h"
#include "../vec2.h"
#include "../animation.h"
#include "../sprite.h"

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

enum struct Barrel_Anim : u32 {
    Idle,
    Destroyed
};

struct Dmg {
    f32       amount;
    Direction came_from_dir;
};

enum struct Barrel_State : u32 { Idle, Destroyed };

struct Entity {
    int health;
    int damage;
    f32 speed;
    f32 x;
    f32 y;
    f32 z;
    f32 x_vel;
    f32 z_vel;

    // used to collect all the received damage in a frame
    std::vector<Dmg> damage_queue;

    f32 sprite_frame_w;
    f32 sprite_frame_h;
    Direction dir;
    Direction dir_prev;

    // this is relative to the player position (which is always considered to
    // be where the center of the bottom border of the drawn sprite is)
    SDL_FRect collision_box_offsets;
    // same thing goes for hurtbox_offsets
    SDL_FRect hurtbox_offsets;
    // same thing goes for hitbox_offsets
    SDL_FRect hitbox_offsets;
    // same thing goes for shadow_offset
    SDL_FRect shadow_offsets;

    const Sprite* sprite;
    Animation     anim;

    // extra data unique to an Entity_Type
    Entity_Type type;
    union {
        struct {
            Player_State state;
        } extra_player;

        struct {} extra_enemy;

        struct {
            Barrel_State state;
        } extra_barrel;
    };
};

// gives coordinates such that when they are used to draw the center point of the bottom border
// of the sprite is at the entity x, y
Vec2<f32> entity_offset_to_bottom_center(const Entity& e);

SDL_FRect entity_get_world_collision_box(const Entity& e);
SDL_FRect entity_get_world_hitbox(const Entity& e);
SDL_FRect entity_get_world_hurtbox(const Entity& e);
