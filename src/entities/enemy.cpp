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
    enemy.hitbox_offsets        = {-sprite_frame_w/6.5f, -20, 2*sprite_frame_w/6.5f, 10};
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

static Direction dir_for_dir_vec(Vec2<f32> dir_vec) {
    if (dir_vec.x > 0) {
        return Direction::Right;
    }
    else {
        return Direction::Left;
    }
}

static void enemy_rotate_towards_player(Entity& e, const Vec2<f32> enemy_pos, const Vec2<f32> player_pos) {
    auto dir = player_pos - enemy_pos;
    e.dir = dir_for_dir_vec(dir);
}

static void enemy_handle_movement(Entity& e, const Entity& player, const Game& g) {
    if (e.health <= 0) return;
    if (e.extra_enemy.slot == Slot::None) return;

    const Vec2<f32> enemy_pos = {e.x, e.y};
    const Vec2<f32> player_pos = {player.x, player.y};
    if (enemy_pos.within_len_from(e.extra_enemy.target_pos, 0.3f)) {
        enemy_rotate_towards_player(e, enemy_pos, player_pos);
        e.x_vel = 0.0f;
        e.y_vel = 0.0f;
        e.extra_enemy.state = Enemy_State::Standing;
    }
    else {
        auto dir = e.extra_enemy.target_pos - enemy_pos;
        e.dir = dir_for_dir_vec(dir);

        dir.normalize();
        e.x_vel = dir.x * e.speed;
        e.y_vel = dir.y * e.speed;
    }

    bool could_go = entity_movement_handle_collisions_and_pos_change(e, &g, collide_opts);
    if (!could_go) {
        enemy_rotate_towards_player(e, enemy_pos, player_pos);
        e.extra_enemy.state = Enemy_State::Standing;
    }
}

static void enemy_receive_damage(Entity& e) {
    if (e.health <= 0.0f) return;
    auto got_hit = false;
    auto hit_type = Hit_Type::Normal;
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

        if (dmg.type != Hit_Type::Normal) {
            hit_type = dmg.type;
        }
    }

    if (got_hit) {
        u32 anim_idx = 0;
        e.extra_enemy.state = Enemy_State::Got_Hit;
        switch (hit_type) {
            case Hit_Type::Normal: {
                if (e.extra_enemy.type == Enemy_Type::Boss) {
                    anim_idx = (u32)Enemy_Boss_Anim::Got_Hit;
                }
                else {
                    anim_idx = (u32)Enemy_Anim::Got_Hit;
                }
                break;
            }

            case Hit_Type::Knockdown: {
                e.extra_enemy.state = Enemy_State::Knocked_Down;
                if (e.extra_enemy.type == Enemy_Type::Boss) {
                    anim_idx = (u32)Enemy_Boss_Anim::Knocked_Down;
                }
                else {
                    anim_idx = (u32)Enemy_Anim::Knocked_Down;
                }
                break;
            }

            case Hit_Type::Special: {
                unreachable("idk what to do here at this point");
            }
        }

        animation_start(
            e.anim,
            { .anim_idx = anim_idx, .frame_duration_ms = 200 }
        );
    }

    e.damage_queue.clear();
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

static void enemy_claim_slot(Entity& e, const Entity& player, Game& g) {
    const auto slots = player.extra_player.slots;
    const auto empty_slot = find_empty_slot(slots);
    if (empty_slot != Slot::None) {
        e.extra_enemy.target_pos = claim_slot_position(g, empty_slot);
        e.extra_enemy.slot = empty_slot;
    }
}

static void enemy_return_claimed_slot(Entity& e, const Entity& player, Game& g) {
    if (e.extra_enemy.slot == Slot::None) unreachable("we shouldnt ever hit this code path if slot is invalid");

    return_claimed_slot(g, e.extra_enemy.slot);
}

static void enemy_update_target_pos(Entity& e, const Entity& player) {
    if (e.extra_enemy.slot == Slot::None) unreachable("we shouldnt ever hit this code path if slot is invalid");

    e.extra_enemy.target_pos = calc_world_coordinates_of_slot({player.x, player.y}, player.extra_player.slots, e.extra_enemy.slot);
}

static bool enemy_can_move(const Entity& e) {
    const auto& s = e.extra_enemy.state;
    return s != Enemy_State::Got_Hit && s != Enemy_State::Knocked_Down && s != Enemy_State::On_The_Ground;
}

Update_Result enemy_update(Entity& e, const Entity& player, Game& g) {
    assert(e.type == Entity_Type::Enemy);

    // TODO: currently the hurtbox isnt rotated, sooooo... Do the same behavior
    // we do for the player with the hurtbox rotation when we do the logic
    // of hitting the player
    //
    // also, enemies should collide with the top wall no matter what, the other walls
    // its probably fine for them not to (bottom and left one especially, maybe they
    // be coming out of that area off screen from the bottom or something)

    animation_update(e.anim);

    if (e.extra_enemy.slot == Slot::None) enemy_claim_slot(e, player, g);
    else enemy_update_target_pos(e, player);

    if (enemy_can_move(e)) {
        enemy_handle_movement(e, player, g);
        enemy_receive_damage(e);
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

        } break;

        case Enemy_State::Running: {
        } break;

        case Enemy_State::Punching: break;
        case Enemy_State::Throwing_Knife: break;

        case Enemy_State::Got_Hit: {
            const auto knockback_finished = enemy_handle_knockback(e, g);
            const auto anim_finished = animation_is_finished(e.anim);
            if (knockback_finished && anim_finished && e.health <= 0.0f) {
                e.extra_enemy.state = Enemy_State::Dying;
                animation_start(
                    e.anim,
                    {
                        .anim_idx          = (u32)Enemy_Anim::Knocked_Down,
                        .frame_duration_ms = 200,
                        .fadeout           = { .enabled = true, .perc_per_sec = 0.05 },
                    }
                );
            }
            else if (knockback_finished && anim_finished) {
                e.extra_enemy.state = Enemy_State::Standing;
            }
        } break;

        case Enemy_State::Knocked_Down: {
            if (animation_is_finished(e.anim)) {
                e.extra_enemy.state = Enemy_State::On_The_Ground;
                animation_start(
                    e.anim,
                    {
                        .anim_idx          = (u32)Enemy_Anim::On_The_Ground,
                        .frame_duration_ms = 750,
                    }
                );
            }
        } break;

        case Enemy_State::On_The_Ground: {
            if (animation_is_finished(e.anim)) {
                e.extra_enemy.state = Enemy_State::Standing;
            }

        } break;

        case Enemy_State::Dying: {
            if (animation_is_finished(e.anim)) {
                if (e.extra_enemy.slot != Slot::None) enemy_return_claimed_slot(e, player, g);
                return Update_Result::Remove_Me;
            }
        } break;

        case Enemy_State::Landing: break;
        case Enemy_State::Kicking: break;
        case Enemy_State::Guarding: break;
        case Enemy_State::Guarding_Running: break;
    }

    return Update_Result::None;
}

