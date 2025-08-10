#pragma once

#include <vector>
#include <span>
#include <array>
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
    Barrel,
    Bullet,
    Collectible,
};

enum struct Collectible_Type {
    Knife,
    Gun,
};

struct Collectible {
    Collectible_Type type;
};

enum struct Player_State {
    Standing,
    Running,
    Attacking,
    Kicking_Drop,
    Got_Hit,
    Dying,
    Takeoff,
    Jumping,
    Landing,
    Picking_Up_Collectible,
};

static_assert((u32)Player_State::Standing == 0);
static_assert((u32)Player_State::Running  == 1);

enum struct Slot { None, Top_Left, Top_Right, Bottom_Left, Bottom_Right };

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
    Standing       = 0,
    Running        = 1,
    Punching_Left  = 2,
    Punching_Right = 3,
    Kicking_Left   = 4,
    Kicking_Right  = 5,
    Kicking_Drop   = 6,
    Got_Hit        = 7,
    Knocked_Down   = 8,
    On_The_Ground  = 9,
    Takeoff        = 10,
    Jumping        = 11,
    Landing        = 12,
    COUNT // keep this last
};

static constexpr u32 sprite_player_frames[]       = { 4, 8, 4, 3, 6, 6, 1, 3, 3, 1, 1, 1, 1 };
static constexpr u32 sprite_knife_player_frames[] = { 4, 8, 0, 3, 0, 0, 1, 0, 0, 0, 1, 1, 1 };
static constexpr u32 sprite_gun_player_frames[]   = { 4, 8, 4, 0, 0, 0, 1, 0, 0, 0, 1, 1, 1 };
static_assert(std::size(sprite_player_frames)       == (u32)Player_Anim::COUNT);
static_assert(std::size(sprite_knife_player_frames) == (u32)Player_Anim::COUNT);
static_assert(std::size(sprite_gun_player_frames)   == (u32)Player_Anim::COUNT);

enum struct Barrel_Anim : u32 {
    Idle      = 0,
    Destroyed = 1,
    COUNT // keep this last
};

static constexpr u32 sprite_barrel_frames[] = { 1, 1 };
static_assert(std::size(sprite_barrel_frames) == (u32)Barrel_Anim::COUNT);

enum struct Hit_Type { Normal, Knockdown, Power };

struct Dmg {
    f32       amount;
    Direction going_to;
    Hit_Type  type;
};

enum struct Barrel_State { Idle, Destroyed };

enum struct Enemy_Type {
    Goon,
    Punk,
    Thug,
    Boss
};

// the value should always correspond to the sprite row for the animation
enum struct Enemy_Anim : u32 {
    Standing      = 0,
    Running       = 1,
    Punch_Left    = 2,
    Punch_Right   = 3,
    Got_Hit       = 4,
    Knocked_Down  = 5,
    On_The_Ground = 6,
    Throw_Knife   = 7,
    Landing       = 8,
    Flying_Back   = 9,
    COUNT // keep this last
};

static constexpr u32 sprite_enemy_frames[]       = { 1, 8, 3, 3, 3, 3, 1, 4, 1, 1 };
static constexpr u32 sprite_knife_enemy_frames[] = { 1, 8, 0, 3, 0, 0, 0, 4, 0, 0 };
static constexpr u32 sprite_gun_enemy_frames[]   = { 1, 8, 4, 0, 0, 0, 0, 0, 0, 0 };
static_assert(std::size(sprite_enemy_frames)       == (u32)Enemy_Anim::COUNT);
static_assert(std::size(sprite_knife_enemy_frames) == (u32)Enemy_Anim::COUNT);
static_assert(std::size(sprite_gun_enemy_frames)   == (u32)Enemy_Anim::COUNT);

// the value should always correspond to the sprite row for the animation
enum struct Enemy_Boss_Anim : u32 {
    Standing       = 0,
    Running        = 1,
    Punch_Left     = 2,
    Punch_Right    = 3,
    Kick           = 4,
    Got_Hit        = 5,
    Knocked_Down   = 6,
    On_The_Ground  = 7,
    Landing        = 8,
    Guard_Standing = 9,
    Guard_Running  = 10,
    Flying_Back    = 11,
    COUNT // keep this last
};

static constexpr u32 sprite_enemy_boss_frames[] = { 1, 8, 4, 3, 5, 2, 3, 1, 1, 1, 8, 1 };
static_assert(std::size(sprite_enemy_boss_frames) == (u32)Enemy_Boss_Anim::COUNT);

enum struct Enemy_State {
    Standing,
    Running,
    Attacking,
    Got_Hit,
    Knocked_Down,
    On_The_Ground,
    Standing_Up,
    Flying_Back,
    Dying,
    Landing,
    Guarding,
    Guarding_Running,
    In_Position_For_Attack,
    Picking_Up_Collectible,
};

struct Handle {
    u32 id;

    bool operator==(const Handle& other) const {
        return id == other.id;
    }

    bool operator!=(const Handle& other) const {
        return id != other.id;
    }
};

enum struct Knife_State {
    Thrown,
    Dropped,
    On_The_Ground,
    Picked_Up,
    Disappearing,
};

enum struct Knife_Anim {
    Thrown  = 0,
    Dropped = 1,
    COUNT // keep this last
};

static constexpr u32 sprite_knife_frames[]   = { 4, 1 };
static_assert(std::size(sprite_knife_frames) == (u32)Knife_Anim::COUNT);

struct Collide_Opts {
    std::span<const Entity_Type> dont_collide_with;
    bool                         collide_with_walls;
    bool                         reset_position_on_wall_impact = true;
};

struct Knife_Thrown_Info {
    Vec2<f32>   position;
    Direction   dir;
    Entity_Type thrown_by;
};

struct Knife_Dropped_Info {
    Vec2<f32>   position;
    Direction   dir;
    Entity_Type dropped_by;
    bool        instantly_disappear;
};

enum struct Gun_State {
    Dropped,
    On_The_Ground,
    Picked_Up,
};

enum struct Gun_Anim {};

struct Entity {
    Handle handle;
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
    // same thing goes for rotation_center_offsets
    SDL_FPoint rotation_center_offsets;
    // same thing goes for bullet_start_offsets
    Vec2<f32> bullet_start_offsets;

    Animation     anim;

    // extra data unique to an Entity_Type
    Entity_Type type;
    union {
        struct {
            Player_State        state;
            Player_Attack_Slots slots;
            u32                 combo;
            u32                 bullets;
            bool                last_attack_successful;
            u64                 last_attack_timestamp; // for resetting combo after some time
            bool                has_knife;
            bool                has_gun;
        } extra_player;

        struct {
            Enemy_Type  type;
            Enemy_State state;
            Vec2<f32>   target_pos;
            Slot        slot;
            u64         last_attack_timestamp;
            u64         ready_to_attack_timestamp;
            u64         idx_attack;
            bool        has_knife;
            bool        can_spawn_knives;
            bool        has_gun;
        } extra_enemy;

        struct {
            Barrel_State state;
        } extra_barrel;

        struct {
            Vec2<f32>    pos_start;
            Vec2<f32>    pos_curr;
            Vec2<f32>    pos_end;
            u64          creation_timestamp;
            u64          time_of_flight_ms;
            f32          thickness;
            f32          length;
        } extra_bullet;

        struct {
            Collectible_Type type;
            bool             picked_up  = false;
            bool             pickupable = true;
            union {
                struct {
                    Knife_State state;
                    Entity_Type created_by;
                    bool        started_going_off_screen;
                    bool        instantly_disappear;
                } knife;

                struct {
                    Gun_State state;
                } gun;
            };
        } extra_collectible;
    };
};

// gives coordinates such that when they are used to draw the center point of the bottom border
// of the sprite is at the entity x, y
Vec2<f32> entity_offset_to_bottom_center(const Entity& e);

Vec2<f32> entity_get_pos(const Entity& e);

SDL_FRect entity_get_world_collision_box(const Entity& e);
SDL_FRect entity_get_world_hitbox(const Entity& e);
SDL_FRect entity_get_world_hurtbox(const Entity& e);

struct Game;
void entity_draw(SDL_Renderer* r, const Entity& e, const Game* g);
void entity_draw_knife(SDL_Renderer* r, const Entity& e, Game* g);
void entity_draw_gun(SDL_Renderer* r, const Entity& e, Game* g);

bool entity_movement_handle_collisions_and_pos_change(Entity& e, const Game* g, Collide_Opts opts = {});
void entity_handle_rotating_offsets(Entity& e);

Slot find_empty_slot(const Player_Attack_Slots& slots);
Vec2<f32> calc_world_coordinates_of_slot(Vec2<f32> player_world_pos, const Player_Attack_Slots& slots, Slot slot);
Vec2<f32> claim_slot_position(Game& game, Slot slot);
void return_claimed_slot(Game& game, Slot slot);

Entity* entity_pickup_collectible(const Entity& e, Game& g);
