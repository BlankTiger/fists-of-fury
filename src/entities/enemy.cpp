#include <cassert>

#include "enemy.h"
#include "../draw.h"
#include "../utils.h"
#include "entity.h"

Entity enemy_init(const Game& g, Enemy_Init_Opts opts) {
    const auto sprite_frame_h = 48;
    const auto sprite_frame_w = 48;
    Entity enemy{};
    enemy.x                     = opts.x;
    enemy.y                     = opts.y;
    enemy.health                = opts.health;
    enemy.damage                = opts.damage;
    enemy.speed                 = opts.speed;
    enemy.type                  = Entity_Type::Enemy;
    enemy.extra_enemy.type      = opts.type;
    enemy.sprite_frame_w        = sprite_frame_w;
    enemy.sprite_frame_h        = sprite_frame_h;
    enemy.collision_box_offsets = {-sprite_frame_w/7,    -3,  2*sprite_frame_w/7,    4};
    enemy.hurtbox_offsets       = {sprite_frame_w/7,     -16, 10,                    6};
    enemy.hitbox_offsets        = {-sprite_frame_w/6.5f, -23, 2*sprite_frame_w/6.5f, 23};
    enemy.shadow_offsets        = {-7,                   -1,  14,                    2};
    enemy.dir                   = Direction::Left;
    enemy.extra_enemy.state     = Enemy_State::Standing;

    switch (opts.type) {
        case Enemy_Type::Goon: {
            enemy.anim.sprite = &g.sprite_enemy_goon;
            animation_start(enemy.anim, { .anim_idx = (u32)Enemy_Anim::Standing, .looping = true });
            break;
        }

        case Enemy_Type::Thug: {
            enemy.anim.sprite = &g.sprite_enemy_thug;
            animation_start(enemy.anim, { .anim_idx = (u32)Enemy_Anim::Standing, .looping = true });
            break;
        }

        case Enemy_Type::Punk: {
            enemy.anim.sprite = &g.sprite_enemy_punk;
            animation_start(enemy.anim, { .anim_idx = (u32)Enemy_Anim::Standing, .looping = true });
            break;
        }

        case Enemy_Type::Boss: {
            enemy.anim.sprite = &g.sprite_enemy_boss;
            animation_start(enemy.anim, { .anim_idx = (u32)Enemy_Boss_Anim::Standing, .looping = true });
            break;
        }
    }

    return enemy;
}

void enemy_draw(SDL_Renderer* r, const Entity& e, const Game& g) {
    entity_draw(r, e, &g);
}

static void handle_movement(Entity& e, const Entity& player, const Game& g) {
    if (e.health <= 0) return;

    auto dir = Vec2{player.x, player.y} - Vec2{e.x, e.y};
    if (dir.x > 0) {
        e.dir = Direction::Right;
    }
    else {
        e.dir = Direction::Left;
    }

    dir.normalize();
    e.x_vel = dir.x * e.speed;
    e.y_vel = dir.y * e.speed;

    entity_movement_handle_collisions_and_pos_change(e, &g);
}

static void receive_damage(Entity& e) {
    if (e.health <= 0.0f) return;
    auto got_hit = false;
    for (auto dmg : e.damage_queue) {
        got_hit = true;
        e.health -= dmg.amount;
        e.y_vel = 0.0f;
        if (dmg.going_to == Direction::Left) {
            e.x_vel = -settings.enemy_knockback_velocity;
        }
        else if (dmg.going_to == Direction::Right) {
            e.x_vel = settings.enemy_knockback_velocity;
        }
        else {
            unreachable("shouldnt ever get a different direction");
        }
    }

    if (got_hit) {
        u32 anim_idx = 0;
        e.extra_enemy.state = Enemy_State::Got_Hit;
        if (e.extra_enemy.type == Enemy_Type::Boss) {
            anim_idx = (u32)Enemy_Boss_Anim::Got_Hit;
        } 
        else {
            anim_idx = (u32)Enemy_Anim::Got_Hit;
        }

        animation_start(
            e.anim,
            { .anim_idx = anim_idx, .frame_duration_ms = 200 }
        );
    }

    e.damage_queue.clear();
}

static bool handle_knockback(Entity& e, const Game& g) {
    entity_movement_handle_collisions_and_pos_change(e, &g);

    if (e.x_vel > 0) e.x_vel -= settings.enemy_friction;
    else             e.x_vel += settings.enemy_friction;

    return e.x_vel == 0.0f;
}

Update_Result enemy_update(Entity& e, const Entity& player, const Game& g) {
    assert(e.type == Entity_Type::Enemy);

    animation_update(e.anim);
    if (e.extra_enemy.state != Enemy_State::Got_Hit) {
        handle_movement(e, player, g);
        receive_damage(e);
    }
    else {
        // this makes it so that when the enemy is in Got_Hit state
        // he doesnt receive more damage
        e.damage_queue.clear();
    }

    switch (e.extra_enemy.state) {
        case Enemy_State::Standing: {
            e.extra_enemy.state = Enemy_State::Running;
            animation_start(
                e.anim,
                {
                    .anim_idx = (u32)Enemy_Anim::Running,
                    .looping = true,
                }
            );

            break;
        }

        case Enemy_State::Running: {
            break;
        }

        case Enemy_State::Punching: break;
        case Enemy_State::Throwing_Knife: break;

        case Enemy_State::Got_Hit: {
            const auto knockback_finished = handle_knockback(e, g);
            const auto anim_finished = animation_is_finished(e.anim);
            if (knockback_finished && anim_finished && e.health <= 0.0f) {
                e.extra_enemy.state = Enemy_State::Dying;
                animation_start(
                    e.anim,
                    { 
                        .anim_idx          = (u32)Enemy_Anim::Dying,
                        .frame_duration_ms = 200,
                        .fadeout           = { .enabled = true, .perc_per_sec = 0.05 },
                    }
                );
            }
            else if (knockback_finished && anim_finished) {
                e.extra_enemy.state = Enemy_State::Standing;
            }

            break;
        }

        case Enemy_State::Dying: {
            if (animation_is_finished(e.anim)) {
                return Update_Result::Remove_Me;
            }
            break;
        }

        case Enemy_State::Landing: break;
        case Enemy_State::Kicking: break;
        case Enemy_State::Guarding: break;
        case Enemy_State::Guarding_Running: break;
    }

    return Update_Result::None;
}

