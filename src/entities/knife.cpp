#include <cassert>
#include <span>

#include "knife.h"
#include "../utils.h"

Entity knife_init(Game& g, Knife_Init_Opts opts) {
    const auto sprite_frame_w = 48;
    const auto sprite_frame_h = 48;
    Entity knife                 = {};
    knife.handle                 = game_generate_entity_handle(g);
    knife.type                   = Entity_Type::Collectible;
    knife.extra_collectible.type = Collectible_Type::Knife;
    knife.x                      = opts.position.x;
    knife.y                      = opts.position.y;
    knife.dir                    = opts.dir;

    knife.sprite_frame_w = sprite_frame_w;
    knife.sprite_frame_h = sprite_frame_h;

    knife.extra_collectible.knife.state = opts.state;
    knife.extra_collectible.knife.created_by = opts.done_by;
    knife.extra_collectible.knife.instantly_disappear = opts.instantly_disappear;

    knife.hurtbox_offsets         = {-4.8f, -sprite_frame_h/2.8f, sprite_frame_w/4.5f, sprite_frame_h/7.0f};
    knife.shadow_offsets          = {-6, 0, 12, 2};
    switch (knife.extra_collectible.knife.state) {
        case Knife_State::Thrown: {
            if (knife.dir == Direction::Right) {
                knife.x_vel = settings.knife_velocity;
                knife.x += 10.0f;
            } else if (knife.dir == Direction::Left) {
                knife.x_vel = -settings.knife_velocity;
                knife.x -= 10.0f;
            }

            // collision offsets are off to the left, because its easier to handle cleanup when knife goes off screen
            // and we dont collide the knife with anything when its flying anyway
            knife.collision_box_offsets = {-18.0f, -sprite_frame_h/3.2f, 1.0f, 1.0f};
            if (knife.dir == Direction::Left) {
                knife.collision_box_offsets.x = -knife.collision_box_offsets.x;
            }
        } break;

        case Knife_State::Dropped: {
            knife.shadow_offsets.y = -7;
            knife.shadow_offsets.x = -3;
            knife.shadow_offsets.w = 6;
            knife.y += 7.0f; // experimentally found offset that looks best for now
            knife.z_vel = settings.knife_drop_jump_velocity;

            if (knife.dir == Direction::Right) {
                knife.x_vel = -settings.knife_drop_sideways_velocity;
            } else if (knife.dir == Direction::Left) {
                knife.x_vel = settings.knife_drop_sideways_velocity;
            }

            knife.collision_box_offsets = knife.hurtbox_offsets;
            knife.collision_box_offsets.y = knife.shadow_offsets.y; 
        } break;

        case Knife_State::Picked_Up:     unreachable("not possible");
        case Knife_State::On_The_Ground: unreachable("not possible");
        case Knife_State::Disappearing:  unreachable("not possible");
    }

    knife.anim.sprite = &g.sprite_knife;

    Anim_Start_Opts anim_opts = {};
    switch (knife.extra_collectible.knife.state) {
        case Knife_State::Thrown: {
            anim_opts.anim_idx = (u32)Knife_Anim::Thrown;
            anim_opts.looping = true;
        } break;

        case Knife_State::Dropped: {
            anim_opts.anim_idx = (u32)Knife_Anim::Dropped;
            anim_opts.rotation = {
                .enabled = true,
                .finish_ranges = {{269, 271}},
                .deg_per_sec = 2300.0f,
                .rotations_min = 0,
            };
        } break;

        case Knife_State::Picked_Up:     unreachable("not possible");
        case Knife_State::On_The_Ground: unreachable("not possible");
        case Knife_State::Disappearing:  unreachable("not possible");
    }
    animation_start(knife.anim, anim_opts);

    return knife;
}

static bool handle_dealing_damage(const Entity& e, Game& g) {
    auto hurtbox = entity_get_world_hurtbox(e);
    bool hit_something = false;

    for (auto& other_e : g.entities) {
        if (other_e.type == Entity_Type::Collectible
            || other_e.type == Entity_Type::Barrel
            || other_e.type == e.extra_collectible.knife.created_by
        ) continue;

        auto hitbox = entity_get_world_hitbox(other_e);
        if (SDL_HasRectIntersectionFloat(&hurtbox, &hitbox)) {
            hit_something = true;
            other_e.damage_queue.push_back({settings.knife_damage, e.dir, Hit_Type::Normal});
            break;
        }
    }

    return hit_something;
}

static bool handle_movement_while_thrown(Entity& e, const Game& g) {
    auto in_bounds = entity_movement_handle_collisions_and_pos_change(e, &g, knife_collide_opts);
    return in_bounds;
}

static bool handle_movement_while_dropped(Entity& e, const Game& g) {
    e.z_vel += settings.gravity * g.dt;
    e.z += e.z_vel * g.dt;
    e.x += e.x_vel * g.dt;

    auto knife_ground_level = settings.ground_level + e.sprite_frame_h / 4.0f;
    if (e.z >= knife_ground_level) {
        e.z = knife_ground_level;
        e.z_vel = 0.0f;
        e.x_vel = 0.0f;

        return true;
    }

    return false;
}

Update_Result knife_update(Entity& e, Game& g) {
    assert(e.type                   == Entity_Type::Collectible);
    assert(e.extra_collectible.type == Collectible_Type::Knife);

    switch (e.extra_collectible.knife.state) {
        case Knife_State::Thrown: {
            auto in_bounds = handle_movement_while_thrown(e, g);
            if (!in_bounds) {
                return Update_Result::Remove_Me;
            }
            auto hit_something = handle_dealing_damage(e, g);
            if (hit_something) {
                return Update_Result::Remove_Me;
            }
        } break;

        case Knife_State::Dropped: {
            auto on_the_ground = handle_movement_while_dropped(e, g);
            if (on_the_ground && animation_is_finished(e.anim)) {
                e.extra_collectible.pickupable = e.extra_collectible.knife.created_by != Entity_Type::Player;
                auto rotation = e.anim.rotation; // preserve the rotation so that we draw the sprite in the correct orientation
                rotation.enabled = false;
                if (e.extra_collectible.knife.instantly_disappear) {
                    e.extra_collectible.knife.state = Knife_State::Disappearing;
                    animation_start(e.anim, {
                        .anim_idx = (u32)Knife_Anim::Dropped,
                        .fadeout = { .enabled = true },
                        .rotation = rotation,
                    });
                } else {
                    e.extra_collectible.knife.state = Knife_State::On_The_Ground;
                    animation_start(e.anim, {
                        .anim_idx = (u32)Knife_Anim::Dropped,
                        .rotation = rotation,
                    });
                }
            }
        } break;

        case Knife_State::On_The_Ground: {
            if (e.extra_collectible.picked_up) {
                return Update_Result::Remove_Me;
            }
        } break;

        case Knife_State::Disappearing: {
            if (animation_is_finished(e.anim)) {
                return Update_Result::Remove_Me;
            }
        } break;

        case Knife_State::Picked_Up: {
            return Update_Result::Remove_Me;
        } break;
    }

    animation_update(e.anim, g.dt, g.dt_real);
    return Update_Result::None;
}

void knife_draw(SDL_Renderer* r, const Entity& e, const Game& g) {
    assert(e.type                   == Entity_Type::Collectible);
    assert(e.extra_collectible.type == Collectible_Type::Knife);

    entity_draw(r, e, &g);
}

void knife_throw(Game& g, const Entity& e) {
    g.knives_thrown_queue.push_back({
        .position = entity_get_pos(e),
        .dir = e.dir,
        .thrown_by = e.type,
    });
}

void knife_drop(Game& g, const Entity& e, Knife_Drop_Opts opts) {
    g.knives_dropped_queue.push_back({
        .position = entity_get_pos(e),
        .dir = e.dir,
        .dropped_by = e.type,
        .instantly_disappear = opts.instantly_disappear,
    });
}
