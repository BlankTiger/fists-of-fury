#include <cassert>

#include "enemy.h"
#include "knife.h"
#include "../draw.h"
#include "../utils.h"
#include "entity.h"

Entity enemy_init(Game& g, Enemy_Init_Opts opts) {
    const auto sprite_frame_h = 48;
    const auto sprite_frame_w = 48;
    Entity enemy{};
    enemy.handle                       = game_generate_entity_handle(g);
    enemy.x                            = opts.x;
    enemy.y                            = opts.y;
    enemy.health                       = opts.health;
    enemy.damage                       = opts.damage;
    enemy.speed                        = opts.speed;
    enemy.type                         = Entity_Type::Enemy;
    enemy.extra_enemy.type             = opts.type;
    enemy.sprite_frame_w               = sprite_frame_w;
    enemy.sprite_frame_h               = sprite_frame_h;
    enemy.collision_box_offsets        = {-sprite_frame_w/7,    -3,  2*sprite_frame_w/7,    4};
    enemy.hurtbox_offsets              = {-sprite_frame_w/7     -    10,                    -14, 10, 6};
    enemy.hitbox_offsets               = {-sprite_frame_w/6.5f, -20, 2*sprite_frame_w/6.5f, 10};
    enemy.shadow_offsets               = {-7,                   -1,  14,                    2};
    enemy.dir                          = Direction::Left;
    enemy.extra_enemy.state            = Enemy_State::Standing;
    enemy.extra_enemy.has_knife        = false;
    enemy.extra_enemy.can_spawn_knives = false;

    switch (opts.type) {
        case Enemy_Type::Goon: {
            enemy.extra_enemy.has_knife = true;
            enemy.extra_enemy.can_spawn_knives = true;
            enemy.anim.sprite = &g.sprite_enemy_goon;
            animation_start(enemy.anim, { .anim_idx = (u32)Enemy_Anim::Standing, .looping = true });
        } break;

        case Enemy_Type::Thug: {
            enemy.anim.sprite = &g.sprite_enemy_thug;
            animation_start(enemy.anim, { .anim_idx = (u32)Enemy_Anim::Standing, .looping = true });
        } break;

        case Enemy_Type::Punk: {
            enemy.anim.sprite = &g.sprite_enemy_punk;
            animation_start(enemy.anim, { .anim_idx = (u32)Enemy_Anim::Standing, .looping = true });
        } break;

        case Enemy_Type::Boss: {
            enemy.anim.sprite = &g.sprite_enemy_boss;
            animation_start(enemy.anim, { .anim_idx = (u32)Enemy_Boss_Anim::Standing, .looping = true });
        } break;
    }

    return enemy;
}

void enemy_draw(SDL_Renderer* r, const Entity& e, Game& g) {
    assert(e.type == Entity_Type::Enemy);
    entity_draw(r, e, &g);
    if (e.extra_enemy.has_knife) entity_draw_knife(r, e, &g);
}

static Anim_Start_Opts enemy_get_anim_knocked_down(const Entity& e) {
    auto anim_idx = (u32)Enemy_Anim::Knocked_Down;
    if (e.extra_enemy.type == Enemy_Type::Boss) anim_idx = (u32)Enemy_Boss_Anim::Knocked_Down;
    return { .anim_idx = anim_idx, .frame_duration_ms = 200 };
}

static Anim_Start_Opts enemy_get_anim_flying_back(const Entity& e) {
    auto anim_idx = (u32)Enemy_Anim::Flying_Back;
    if (e.extra_enemy.type == Enemy_Type::Boss) anim_idx = (u32)Enemy_Boss_Anim::Flying_Back;
    return { .anim_idx = anim_idx, .looping = true };
}

static Anim_Start_Opts enemy_get_anim_got_hit(const Entity& e) {
    auto anim_idx = (u32)Enemy_Anim::Got_Hit;
    if (e.extra_enemy.type == Enemy_Type::Boss) {
        anim_idx = (u32)Enemy_Boss_Anim::Got_Hit;
    }
    return { .anim_idx = anim_idx, .frame_duration_ms = 75 };
}

static Anim_Start_Opts enemy_get_anim_standing(const Entity& e) {
    auto anim_idx = (u32)Enemy_Anim::Standing;
    if (e.extra_enemy.type == Enemy_Type::Boss) {
        anim_idx = (u32)Enemy_Boss_Anim::Standing;
    }
    return { .anim_idx = anim_idx, .frame_duration_ms = 75, .looping = true };
}

static Anim_Start_Opts enemy_get_anim_running(const Entity& e) {
    auto anim_idx = (u32)Enemy_Anim::Running;
    if (e.extra_enemy.type == Enemy_Type::Boss) {
        anim_idx = (u32)Enemy_Boss_Anim::Running;
    }
    return { .anim_idx = anim_idx, .frame_duration_ms = 75, .looping = true };
}

static Anim_Start_Opts enemy_get_anim_punch_left(const Entity& e) {
    auto anim_idx = (u32)Enemy_Anim::Punch_Left;
    if (e.extra_enemy.type == Enemy_Type::Boss) {
        anim_idx = (u32)Enemy_Boss_Anim::Punch_Left;
    }
    return { .anim_idx = anim_idx, .frame_duration_ms = 90 };
}

static Anim_Start_Opts enemy_get_anim_punch_right(const Entity& e) {
    auto anim_idx = (u32)Enemy_Anim::Punch_Right;
    if (e.extra_enemy.type == Enemy_Type::Boss) {
        anim_idx = (u32)Enemy_Boss_Anim::Punch_Right;
    }
    return { .anim_idx = anim_idx, .frame_duration_ms = 100 };
}

static Direction dir_for_dir_vec(Vec2<f32> dir_vec) {
    if (dir_vec.x > 0) {
        return Direction::Right;
    } else {
        return Direction::Left;
    }
}

static void enemy_rotate_towards_player(Entity& e, const Vec2<f32> enemy_pos, const Vec2<f32> player_pos) {
    auto dir = player_pos - enemy_pos;
    e.dir = dir_for_dir_vec(dir);
}

static void enemy_make_stationary(Entity& e) {
    e.x_vel = 0.0f;
    e.y_vel = 0.0f;
}

static void enemy_stand(Entity& e) {
    e.extra_enemy.state = Enemy_State::Standing;
    auto opts = enemy_get_anim_standing(e);
    animation_start(e.anim, opts);
}

static void enemy_run(Entity& e) {
    e.extra_enemy.state = Enemy_State::Running;
    auto opts = enemy_get_anim_running(e);
    animation_start(e.anim, opts);
}

static bool enemy_is_close_to_target_pos(const Entity& e) {
    auto enemy_pos = entity_get_pos(e);
    return enemy_pos.within_len_from(e.extra_enemy.target_pos, 0.3f);
}

static void enemy_get_ready_to_attack(Entity& e, const Game& g) {
    e.extra_enemy.ready_to_attack_timestamp = g.time_ms;
    e.extra_enemy.state = Enemy_State::In_Position_For_Attack;
}

static void enemy_handle_movement(Entity& e, const Entity& player, const Game& g) {
    if (e.health <= 0) return;
    if (e.extra_enemy.slot == Slot::None) return;

    const auto enemy_pos = entity_get_pos(e);
    const auto player_pos = entity_get_pos(player);
    if (enemy_is_close_to_target_pos(e)) {
        enemy_rotate_towards_player(e, enemy_pos, player_pos);
        enemy_make_stationary(e);
        if (e.extra_enemy.state != Enemy_State::Standing) enemy_stand(e);
        enemy_get_ready_to_attack(e, g);
    } else {
        auto dir = e.extra_enemy.target_pos - enemy_pos;
        e.dir = dir_for_dir_vec(dir);
        if (e.extra_enemy.has_knife) {
            enemy_rotate_towards_player(e, enemy_pos, player_pos);
        }

        dir.normalize();
        e.x_vel = dir.x * e.speed;
        e.y_vel = dir.y * e.speed;
    }

    bool could_go = entity_movement_handle_collisions_and_pos_change(e, &g, collide_opts);
    if (!could_go) {
        enemy_rotate_towards_player(e, enemy_pos, player_pos);
        enemy_stand(e);
    }

    entity_handle_rotating_hurtbox(e);
}

// returns wheter got hit
static bool enemy_receive_damage(Entity& e) {
    if (e.health <= 0.0f) return false;
    auto got_hit = false;
    Dmg most_significant_dmg = {};
    for (auto dmg : e.damage_queue) {
        got_hit = true;
        e.health -= dmg.amount;
        e.y_vel = 0.0f;
        if (dmg.going_to == Direction::Left) {
            e.x_vel = -settings.enemy_knockback_velocity;
        } else if (dmg.going_to == Direction::Right) {
            e.x_vel = settings.enemy_knockback_velocity;
        } else {
            unreachable("shouldnt ever get a different direction");
        }

        if (dmg.type != Hit_Type::Normal) {
            most_significant_dmg = dmg;
        }
    }

    if (got_hit) {
        e.extra_enemy.state = Enemy_State::Got_Hit;
        e.extra_enemy.can_spawn_knives = false;

        switch (most_significant_dmg.type) {
            case Hit_Type::Normal: {
                auto opts = enemy_get_anim_got_hit(e);
                animation_start(e.anim, opts);
            } break;

            case Hit_Type::Knockdown: {
                e.z_vel = -settings.enemy_knockdown_velocity;

                if (most_significant_dmg.going_to == Direction::Left) {
                    e.x_vel = -settings.enemy_knockdown_velocity;
                } else if (most_significant_dmg.going_to == Direction::Right) {
                    e.x_vel = settings.enemy_knockdown_velocity;
                }

                e.extra_enemy.state = Enemy_State::Knocked_Down;
                auto opts = enemy_get_anim_knocked_down(e);
                animation_start(e.anim, opts);
            } break;

            case Hit_Type::Power: {
                if (most_significant_dmg.going_to == Direction::Left) {
                    e.x_vel = -settings.enemy_flying_back_velocity;
                } else if (most_significant_dmg.going_to == Direction::Right) {
                    e.x_vel = settings.enemy_flying_back_velocity;
                }

                e.extra_enemy.state = Enemy_State::Flying_Back;
                auto opts = enemy_get_anim_flying_back(e);
                animation_start(e.anim, opts);
            } break;
        }
    }

    e.damage_queue.clear();
    return got_hit;
}

static bool enemy_handle_knockback(Entity& e, const Game& g) {
    entity_movement_handle_collisions_and_pos_change(e, &g, collide_opts);

    if (e.x_vel > 0) {
        e.x_vel -= settings.enemy_friction;
        if (e.x_vel < 0) e.x_vel = 0;
    }
    else if (e.x_vel < 0) {
        e.x_vel += settings.enemy_friction;
        if (e.x_vel > 0) e.x_vel = 0;
    }

    return e.x_vel == 0.0f;
}

static void enemy_handle_flying_back_collateral_dmg(Entity& e, Game& g) {
    SDL_FRect hitbox_box = entity_get_world_hitbox(e);

    for (auto& other_e : g.entities) {
        if (e.handle == other_e.handle) continue;

        SDL_FRect other_e_hitbox = entity_get_world_hitbox(other_e);
        if (SDL_HasRectIntersectionFloat(&hitbox_box, &other_e_hitbox)) {
            auto dir = Direction::Left;
            if (e.dir == Direction::Left) {
                dir = Direction::Right;
            }
            else if (e.dir == Direction::Right) {
                dir = Direction::Left;
            }
            else {
                unreachable("shouldnt ever get a different direction here in this game");
            }

            other_e.damage_queue.push_back({settings.enemy_flying_back_dmg_collateral_dmg, dir, Hit_Type::Knockdown});
        }
    }
}

static bool enemy_handle_flying_back(Entity& e, const Game& g) {
    auto new_collide_opts = collide_opts;
    new_collide_opts.collide_with_walls = true;
    auto in_bounds = entity_movement_handle_collisions_and_pos_change(e, &g, new_collide_opts);
    return !in_bounds;
}

static void enemy_claim_slot(Entity& e, const Entity& player, Game& g) {
    const auto slots = player.extra_player.slots;
    const auto empty_slot = find_empty_slot(slots);
    if (empty_slot != Slot::None) {
        e.extra_enemy.target_pos = claim_slot_position(g, empty_slot);
        e.extra_enemy.slot = empty_slot;
        enemy_run(e);
    }
}

static void enemy_return_claimed_slot(Entity& e, Game& g) {
    if (e.extra_enemy.slot == Slot::None) unreachable("we shouldnt ever hit this code path if slot is invalid");

    return_claimed_slot(g, e.extra_enemy.slot);
}

static void enemy_update_target_pos(Entity& e, const Entity& player, const Game& g) {
    if (e.extra_enemy.has_knife) {
        Vec2<f32> target_pos = {};
        target_pos.y = player.y;
        auto offset_to_be_fully_visible = 12;
        if (e.x > player.x) {
            auto border_x = game_get_border_x(g, Border::Right);
            target_pos.x = border_x - offset_to_be_fully_visible;
        } else {
            auto border_x = game_get_border_x(g, Border::Left);
            target_pos.x = border_x + offset_to_be_fully_visible;
        }
        e.extra_enemy.target_pos = target_pos;
        return;
    }

    if (e.extra_enemy.slot != Slot::None) {
        e.extra_enemy.target_pos = calc_world_coordinates_of_slot({player.x, player.y}, player.extra_player.slots, e.extra_enemy.slot);
        return;
    }
}

static bool enemy_can_move(const Entity& e) {
    const auto& s = e.extra_enemy.state;
    return s != Enemy_State::Got_Hit
        && s != Enemy_State::Knocked_Down
        && s != Enemy_State::On_The_Ground
        && s != Enemy_State::Flying_Back
        && s != Enemy_State::Standing_Up
        && s != Enemy_State::In_Position_For_Attack
        && s != Enemy_State::Attacking;
}

static bool enemy_can_receive_damage(const Entity& e) {
    const auto& s = e.extra_enemy.state;
    return s == Enemy_State::Standing
        || s == Enemy_State::Running
        || s == Enemy_State::Landing
        || s == Enemy_State::In_Position_For_Attack
        || s == Enemy_State::Attacking;
}

static bool enemy_is_moving(const Entity& e) {
    return e.x_vel > 0.0f
        || e.x_vel < 0.0f
        || e.y_vel > 0.0f
        || e.y_vel < 0.0f;
}

static bool enemy_attack_timed_out(const Entity& e, const Game& g) {
    return g.time_ms - e.extra_enemy.last_attack_timestamp > settings.enemy_attack_timeout_ms;
}

static bool enemy_can_attack(const Entity& e, const Game& g) {
    return enemy_is_close_to_target_pos(e) && enemy_attack_timed_out(e, g);
}

static void enemy_deal_damage(Entity& e, Game& g) {
    auto& player = game_get_player_mutable(g);
    auto player_hitbox = entity_get_world_hitbox(player);
    auto enemy_hurtbox = entity_get_world_hurtbox(e);
    if (SDL_HasRectIntersectionFloat(&player_hitbox, &enemy_hurtbox)) {
        player.damage_queue.push_back({e.damage, e.dir, Hit_Type::Normal});
    }
}

static void enemy_attack(Entity& e, Game& g) {
    e.extra_enemy.state = Enemy_State::Attacking;
    Anim_Start_Opts opts = {};
    if (e.extra_enemy.has_knife) {
        opts = enemy_get_anim_punch_right(e);
        animation_start(e.anim, opts);
        e.extra_enemy.has_knife = false;
        knife_throw(g, e);
        return;
    }

    auto attack_anim = e.extra_enemy.idx_attack % 2;
    switch (attack_anim) {
        case 0: {
            opts = enemy_get_anim_punch_left(e);
        } break;

        case 1: {
            opts = enemy_get_anim_punch_right(e);
        } break;
    }

    enemy_deal_damage(e, g);

    animation_start(e.anim, opts);
    e.extra_enemy.idx_attack++;
    e.extra_enemy.last_attack_timestamp = g.time_ms;
}

static void enemy_drop_knife(Entity& e, Game& g) {
    if (e.extra_enemy.has_knife) {
        knife_drop(g, e);
        e.extra_enemy.has_knife = false;
    }
}

static void enemy_respawn_knife(Entity& e, const Game& g) {
    if (e.extra_enemy.has_knife)         return;
    if (!e.extra_enemy.can_spawn_knives) return;

    if (enemy_attack_timed_out(e, g)) {
        e.extra_enemy.has_knife = true;
    }
}

Update_Result enemy_update(Entity& e, const Entity& player, Game& g) {
    assert(e.type == Entity_Type::Enemy);

    animation_update(e.anim, g.dt, g.dt_real);

    if (e.extra_enemy.slot == Slot::None) enemy_claim_slot(e, player, g);

    enemy_update_target_pos(e, player, g);

    if (enemy_can_move(e)) {
        enemy_handle_movement(e, player, g);
    }

    if (enemy_can_receive_damage(e)) {
        auto got_hit = enemy_receive_damage(e);
        if (got_hit) enemy_drop_knife(e, g);
    } else {
        // this makes it so that when the enemy is in Got_Hit state
        // he doesnt receive more damage
        e.damage_queue.clear();
    }

    enemy_respawn_knife(e, g);

    switch (e.extra_enemy.state) {
        case Enemy_State::Standing: {
            if (enemy_is_moving(e)) {
                enemy_run(e);
            } else if (enemy_can_attack(e, g)) {
                enemy_attack(e, g);
            }
        } break;

        case Enemy_State::Running: {
            if (!enemy_is_moving(e)) {
                enemy_stand(e);
            }
        } break;


        case Enemy_State::Attacking: {
            if (animation_is_finished(e.anim)) {
                enemy_stand(e);
            }
        } break;

        case Enemy_State::Got_Hit: {
            const auto knockback_finished = enemy_handle_knockback(e, g);
            const auto anim_finished = animation_is_finished(e.anim);
            if (knockback_finished && anim_finished && e.health <= 0.0f) {
                auto opts = enemy_get_anim_knocked_down(e);
                opts.fadeout = { .enabled = true, .perc_per_sec = 0.8f };
                e.extra_enemy.state = Enemy_State::Dying;
                animation_start(e.anim, opts);
            } else if (knockback_finished && anim_finished) {
                enemy_stand(e);
            }

            if (e.extra_enemy.has_knife) {
                enemy_drop_knife(e, g);
            }
        } break;

        case Enemy_State::Flying_Back: {
            enemy_handle_flying_back_collateral_dmg(e, g);
            const auto hit_wall = enemy_handle_flying_back(e, g);
            // make sure that this path doesnt let the enemy live even tho he has 0hp
            if (hit_wall) {
                // this makes them bounce off of the wall
                e.x_vel = -e.x_vel;
                e.extra_enemy.state = Enemy_State::Knocked_Down;
                auto opts = enemy_get_anim_knocked_down(e);
                animation_start(e.anim, opts);
            }
        } break;

        case Enemy_State::Knocked_Down: {
            auto anim_idx = (u32)Enemy_Anim::On_The_Ground;
            if (e.extra_enemy.type == Enemy_Type::Boss) {
                anim_idx = (u32)Enemy_Boss_Anim::On_The_Ground;
            }

            const auto knockback_finished = enemy_handle_knockback(e, g);
            const auto anim_finished = animation_is_finished(e.anim);

            if (knockback_finished && anim_finished && e.health <= 0.0f) {
                e.extra_enemy.state = Enemy_State::Dying;

                animation_start(
                    e.anim,
                    {
                        .anim_idx          = anim_idx,
                        .frame_duration_ms = 200,
                        .fadeout           = { .enabled = true, .perc_per_sec = 0.8f },
                    }
                );
            }
            else if (knockback_finished && anim_finished) {
                e.extra_enemy.state = Enemy_State::On_The_Ground;

                animation_start(
                    e.anim,
                    {
                        .anim_idx          = anim_idx,
                        .frame_duration_ms = 750,
                    }
                );
            }
        } break;

        case Enemy_State::On_The_Ground: {
            if (animation_is_finished(e.anim)) {
                e.extra_enemy.state = Enemy_State::Standing_Up;
                auto anim_idx = (u32)Enemy_Anim::Landing;
                if (e.extra_enemy.type == Enemy_Type::Boss) {
                    anim_idx = (u32)Enemy_Boss_Anim::Landing;
                }
                animation_start(
                    e.anim,
                    {
                        .anim_idx = anim_idx,
                        .frame_duration_ms = 500,
                    }
                );
            }
        } break;

        case Enemy_State::Standing_Up: {
            if (animation_is_finished(e.anim)) {
                enemy_stand(e);
            }
        } break;

        case Enemy_State::Dying: {
            if (animation_is_finished(e.anim)) {
                if (e.extra_enemy.slot != Slot::None) enemy_return_claimed_slot(e, g);
                return Update_Result::Remove_Me;
            }
        } break;

        case Enemy_State::In_Position_For_Attack: {
            if (g.time_ms - e.extra_enemy.ready_to_attack_timestamp > settings.enemy_attack_timeout_ms) {
                enemy_attack(e, g);
            }
        } break;

        case Enemy_State::Landing: break;
        case Enemy_State::Guarding: break;
        case Enemy_State::Guarding_Running: break;
    }

    return Update_Result::None;
}

