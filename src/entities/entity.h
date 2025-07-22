#pragma once

#include <vector>
#include <span>
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

enum struct Entity_Type {
    Player,
    Enemy,
    Barrel
};

enum struct Player_State {
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

enum struct Slot { Top_Left, Top_Right, Bottom_Left, Bottom_Right };

struct Player_Attack_Slots {
    Vec2<f32> offset_top_left;
    Vec2<f32> offset_top_right;
    Vec2<f32> offset_bottom_left;
    Vec2<f32> offset_bottom_right;

    bool top_left_free;
    bool top_right_free;
    bool bottom_left_free;
    bool bottom_right_free;
};

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
    Direction going_to;
};

enum struct Barrel_State { Idle, Destroyed };

enum struct Enemy_Type {
    Goon,
    Punk,
    Thug,
    Boss
};

enum struct Enemy_Anim : u32 {
    Standing,
    Running,
    Punch_Left,
    Punch_Right,
    Got_Hit,
    Dying,
    Throw_Knife,
    Landing
};

enum struct Enemy_Boss_Anim : u32 {
    Standing,
    Running,
    Punch_Left,
    Punch_Right,
    Kick,
    Got_Hit,
    Dying,
    Landing,
    Guard_Standing,
    Guard_Running
};

enum struct Enemy_State {
    Standing,
    Running,
    Punching,
    Kicking,
    Throwing_Knife,
    Got_Hit,
    Dying,
    Landing,
    Guarding,
    Guarding_Running
};

struct Entity {
    f32 health;
    f32 damage;
    f32 speed;
    f32 x;
    f32 y;
    f32 z;
    f32 x_vel;
    f32 y_vel;
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

    Animation     anim;

    // extra data unique to an Entity_Type
    Entity_Type type;
    union {
        struct {
            Player_State        state;
            Player_Attack_Slots slots;
        } extra_player;

        struct {
            Enemy_Type  type;
            Enemy_State state;
            Vec2<f32>   target_pos;
            Slot        slot;
        } extra_enemy;

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

struct Game;
void entity_draw(SDL_Renderer* r, const Entity& e, const Game* g);

struct Collide_Opts {
    std::span<const Entity_Type> dont_collide_with;
};

void entity_movement_handle_collisions_and_pos_change(Entity& e, const Game* g, Collide_Opts opts = {});

Vec2<f32> claim_slot_position(Game& game, Slot slot);
