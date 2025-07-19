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

    switch (opts.type) {
        case Enemy_Type::Goon: {
            enemy.anim.sprite = &g.sprite_enemy_goon;
            enemy.extra_enemy.state.s_other = Enemy_State::Standing;
            animation_start(enemy.anim, { .anim_idx = (u32)Enemy_Anim::Standing, .looping = true });
            break;
        }

        case Enemy_Type::Thug: {
            enemy.anim.sprite = &g.sprite_enemy_thug;
            enemy.extra_enemy.state.s_other = Enemy_State::Standing;
            animation_start(enemy.anim, { .anim_idx = (u32)Enemy_Anim::Standing, .looping = true });
            break;
        }

        case Enemy_Type::Punk: {
            enemy.anim.sprite = &g.sprite_enemy_punk;
            enemy.extra_enemy.state.s_other = Enemy_State::Standing;
            animation_start(enemy.anim, { .anim_idx = (u32)Enemy_Anim::Standing, .looping = true });
            break;
        }

        case Enemy_Type::Boss: {
            enemy.anim.sprite = &g.sprite_enemy_boss;
            enemy.extra_enemy.state.s_boss = Enemy_Boss_State::Standing;
            animation_start(enemy.anim, { .anim_idx = (u32)Enemy_Boss_Anim::Standing, .looping = true });
            break;
        }
    }

    return enemy;
}

void enemy_draw(SDL_Renderer* r, const Entity& e, const Game& g) {
    entity_draw(r, e, &g);
}

static void go_towards_player(Entity& e, const Entity& player, const Game& g) {
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
    }

    if (got_hit) {
        u32 anim_idx = 0;
        if (e.extra_enemy.type == Enemy_Type::Boss) {
            e.extra_enemy.state.s_boss = Enemy_Boss_State::Got_Hit;
            anim_idx = (u32)Enemy_Boss_Anim::Got_Hit;
        } 
        else {
            e.extra_enemy.state.s_other = Enemy_State::Got_Hit;
            anim_idx = (u32)Enemy_Anim::Got_Hit;
        }

        animation_start(
            e.anim,
            { .anim_idx = anim_idx, .frame_duration_ms = 200 }
        );
    }

    e.damage_queue.clear();
}

Update_Result enemy_update(Entity& e, const Entity& player, const Game& g) {
    assert(e.type == Entity_Type::Enemy);

    animation_update(e.anim);
    go_towards_player(e, player, g);
    receive_damage(e);

    switch (e.extra_enemy.type) {
        case Enemy_Type::Goon: // fallthrough
        case Enemy_Type::Thug: // fallthrough
        case Enemy_Type::Punk: {
            switch (e.extra_enemy.state.s_other) {
                case Enemy_State::Standing: {
                    e.extra_enemy.state.s_other = Enemy_State::Running;
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
                    const auto anim_finished = animation_is_finished(e.anim);
                    if (anim_finished && e.health <= 0.0f) {
                        e.extra_enemy.state.s_other = Enemy_State::Dying;
                        animation_start(
                            e.anim,
                            { 
                                .anim_idx          = (u32)Enemy_Anim::Dying,
                                .frame_duration_ms = 200,
                                .fadeout           = { .enabled = true, .perc_per_sec = 0.05 },
                            }
                        );
                    }
                    else if (anim_finished) {
                        e.extra_enemy.state.s_other = Enemy_State::Standing;
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
            }

            break;
        }

        case Enemy_Type::Boss: {
            switch (e.extra_enemy.state.s_boss) {
                case Enemy_Boss_State::Standing:
                case Enemy_Boss_State::Running:
                case Enemy_Boss_State::Punching:
                case Enemy_Boss_State::Kicking:
                case Enemy_Boss_State::Throwing_Knife:
                case Enemy_Boss_State::Got_Hit:
                case Enemy_Boss_State::Dying:
                case Enemy_Boss_State::Landing:
                case Enemy_Boss_State::Guarding:
                case Enemy_Boss_State::Guarding_Running:
                    unreachable("unimplemented");
            }

            break;
        }
    }

    return Update_Result::None;
}

