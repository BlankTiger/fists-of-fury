#include <cassert>
#include <span>

#include "collectible.h"
#include "../utils.h"

Entity collectible_init(Game& g, Collectible_Init_Opts opts) {
    const auto sprite_frame_w = 16;
    const auto sprite_frame_h = 16;
    Entity collectible                 = {};
    collectible.handle                 = game_generate_entity_handle(g);
    collectible.type                   = Entity_Type::Collectible;
    collectible.extra_collectible.type = opts.type;
    collectible.x                      = opts.position.x;
    collectible.y                      = opts.position.y;
    collectible.dir                    = opts.dir;

    collectible.sprite_frame_w = sprite_frame_w;
    collectible.sprite_frame_h = sprite_frame_h;

    collectible.extra_collectible.state = opts.state;
    collectible.extra_collectible.created_by = opts.done_by;
    collectible.extra_collectible.instantly_disappear = opts.instantly_disappear;

    collectible.hurtbox_offsets         = {-4.8f, -sprite_frame_h/2.8f, sprite_frame_w/4.5f, sprite_frame_h/7.0f};
    collectible.shadow_offsets          = {-6, 0, 12, 2};

    switch (collectible.extra_collectible.state) {
        case Collectible_State::Thrown: {
            if (collectible.dir == Direction::Right) {
                collectible.x_vel = settings.collectible_velocity;
                collectible.x += 10.0f;
            } else if (collectible.dir == Direction::Left) {
                collectible.x_vel = -settings.collectible_velocity;
                collectible.x -= 10.0f;
            }

            // collision offsets are off to the left, because its easier to handle cleanup when collectible goes off screen
            // and we dont collide the collectible with anything when its flying anyway
            collectible.collision_box_offsets = {-18.0f, -sprite_frame_h/3.2f, 1.0f, 1.0f};
            if (collectible.dir == Direction::Left) {
                collectible.collision_box_offsets.x = -collectible.collision_box_offsets.x;
            }
        } break;

        case Collectible_State::Dropped: {
            collectible.collision_box_offsets = collectible.hurtbox_offsets;
            collectible.collision_box_offsets.y = -5.0f;
            collectible.collision_box_offsets.h = 6.0f;

            switch (opts.type) {
                case (Collectible_Type::Knife): {
                    collectible.shadow_offsets.y = -7;
                    collectible.shadow_offsets.x = -3;
                    collectible.shadow_offsets.w = 6;
                    collectible.collision_box_offsets.x = -3.0f;
                    collectible.collision_box_offsets.w = 6.0f;
                    collectible.y += 7.0f; // experimentally found offset that looks best for now
                } break;

                case (Collectible_Type::Gun): {
                    collectible.shadow_offsets.y = 0;
                    collectible.shadow_offsets.x = -7;
                    collectible.shadow_offsets.w = 14;
                    collectible.collision_box_offsets.x = -8.0f;
                    collectible.collision_box_offsets.w = 16.0f;
                    collectible.y += 3.0f; // experimentally found offset that looks best for now
                } break;

                case (Collectible_Type::Food): {
                    collectible.shadow_offsets.y = 0;
                    collectible.shadow_offsets.x = -7;
                    collectible.shadow_offsets.w = 14;
                    collectible.collision_box_offsets.x = -8.0f;
                    collectible.collision_box_offsets.w = 16.0f;
                    collectible.y += 3.0f; // experimentally found offset that looks best for now
                }
            }
            collectible.z_vel = settings.collectible_drop_jump_velocity;

            if (collectible.dir == Direction::Right) {
                collectible.x_vel = -settings.collectible_drop_sideways_velocity;
            } else if (collectible.dir == Direction::Left) {
                collectible.x_vel = settings.collectible_drop_sideways_velocity;
            }
        } break;

        case Collectible_State::Picked_Up:     unreachable("not possible");
        case Collectible_State::On_The_Ground: unreachable("not possible");
        case Collectible_State::Disappearing:  unreachable("not possible");
    }

    switch (opts.type) {
        case Collectible_Type::Knife: {
            collectible.anim.sprite = &g.sprite_knife;
        } break;

        case Collectible_Type::Gun: {
            collectible.anim.sprite = &g.sprite_gun;
        } break;

        case Collectible_Type::Food: {
            collectible.anim.sprite = &g.sprite_food;
        } break;
    }

    Anim_Start_Opts anim_opts = {};
    switch (collectible.extra_collectible.state) {
        case Collectible_State::Thrown: {
            anim_opts.anim_idx = (u32)Collectible_Anim::Normal;
            anim_opts.looping = true;

            switch (opts.type) {
                case Collectible_Type::Knife: {

                } break;

                case Collectible_Type::Gun: {
                    anim_opts.rotation = {
                        .enabled = true,
                        .deg_per_sec = 1200.0f,
                    };
                } break;

                case Collectible_Type::Food: {
                    anim_opts.rotation = {
                        .enabled = true,
                        .deg_per_sec = 1000.0f,
                    };
                } break;
            }
        } break;

        case Collectible_State::Dropped: {
            anim_opts.anim_idx = (u32)Collectible_Anim::Normal;

            std::vector<Rotation_Range> range;
            f32 deg_per_sec;

            switch (opts.type) {
                case Collectible_Type::Knife: {
                    if      (opts.dir == Direction::Left)  range = {{269, 271}};
                    else if (opts.dir == Direction::Right) range = {{89, 91}};
                    else    unreachable("not possible");
                    deg_per_sec = 2300.0f;
                } break;

                case Collectible_Type::Gun: {
                    range = {{0, 10}};
                    deg_per_sec = 1200.0f;
                } break;

                case Collectible_Type::Food: {
                    range = {{0, 10}};
                    deg_per_sec = 800.0f;
                }
            }

            anim_opts.rotation = {
                .enabled = true,
                .finish_ranges = range,
                .deg_per_sec = deg_per_sec,
                .rotations_min = 0,
            };
        } break;

        case Collectible_State::Picked_Up:     unreachable("not possible");
        case Collectible_State::On_The_Ground: unreachable("not possible");
        case Collectible_State::Disappearing:  unreachable("not possible");
    }
    animation_start(collectible.anim, anim_opts);

    return collectible;
}

static bool handle_dealing_damage(const Entity& e, Game& g) {
    auto hurtbox = entity_get_world_hurtbox(e);
    bool hit_something = false;

    for (auto& other_e : g.entities) {
        if (other_e.type == Entity_Type::Collectible
            || other_e.type == Entity_Type::Barrel
            || other_e.type == e.extra_collectible.created_by
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

Update_Result collectible_update(Entity& e, Game& g) {
    assert(e.type == Entity_Type::Collectible);

    switch (e.extra_collectible.state) {
        case Collectible_State::Thrown: {
            auto in_bounds = handle_movement_while_thrown(e, g);
            if (!in_bounds) {
                return Update_Result::Remove_Me;
            }
            auto hit_something = handle_dealing_damage(e, g);
            if (hit_something) {
                return Update_Result::Remove_Me;
            }
        } break;

        case Collectible_State::Dropped: {
            auto on_the_ground = handle_movement_while_dropped(e, g);
            if (on_the_ground && animation_is_finished(e.anim)) {
                e.extra_collectible.pickupable = e.extra_collectible.created_by != Entity_Type::Player;
                auto rotation = e.anim.rotation; // preserve the rotation so that we draw the sprite in the correct orientation
                rotation.enabled = false;
                if (e.extra_collectible.instantly_disappear) {
                    e.extra_collectible.state = Collectible_State::Disappearing;
                    animation_start(e.anim, {
                        .anim_idx = (u32)Collectible_Anim::Normal,
                        .fadeout = { .enabled = true },
                        .rotation = rotation,
                    });
                } else {
                    e.extra_collectible.state = Collectible_State::On_The_Ground;
                    animation_start(e.anim, {
                        .anim_idx = (u32)Collectible_Anim::Normal,
                        .rotation = rotation,
                    });
                }
            }
        } break;

        case Collectible_State::On_The_Ground: {
            if (e.extra_collectible.picked_up) {
                return Update_Result::Remove_Me;
            }
        } break;

        case Collectible_State::Disappearing: {
            if (animation_is_finished(e.anim)) {
                return Update_Result::Remove_Me;
            }
        } break;

        case Collectible_State::Picked_Up: {
            return Update_Result::Remove_Me;
        } break;
    }

    animation_update(e.anim, g.dt, g.dt_real);
    return Update_Result::None;
}

void collectible_draw(SDL_Renderer* r, const Entity& e, const Game& g) {
    assert(e.type == Entity_Type::Collectible);

    entity_draw(r, e, &g);
}

void collectible_throw(Collectible_Type type, Game& g, const Entity& e) {
    auto pos = entity_get_pos(e);
    // offset for it to appear like its thrown from a hand
    pos.x += 1.0f;
    pos.y -= 5.0f;
    g.props_thrown_queue.push_back({
        .type = type,
        .position = pos,
        .dir = e.dir,
        .thrown_by = e.type,
    });
}

void collectible_drop(Collectible_Type type, Game& g, const Entity& e, Collectible_Drop_Opts opts) {
    auto pos = entity_get_pos(e);
    // offset for it to appear like its dropped from a hand
    pos.y -= 7.0f;
    g.props_dropped_queue.push_back({
        .type = type,
        .position = pos,
        .dir = e.dir,
        .dropped_by = e.type,
        .instantly_disappear = opts.instantly_disappear,
    });
}
