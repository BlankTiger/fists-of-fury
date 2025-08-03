#include <cassert>

#include "entity.h"
#include "../settings.h"
#include "../draw.h"
#include "../utils.h"
#include "../game.h"

Vec2<f32> entity_offset_to_bottom_center(const Entity& e) {
    return {e.x - e.sprite_frame_w / 2, e.y - e.sprite_frame_h};
}

Vec2<f32> entity_get_pos(const Entity& e) {
    return {e.x, e.y};
}

SDL_FRect entity_get_world_collision_box(const Entity& e) {
    return {
        e.x + e.collision_box_offsets.x,
        e.y + e.collision_box_offsets.y,
        e.collision_box_offsets.w,
        e.collision_box_offsets.h
    };
}

SDL_FRect entity_get_world_hurtbox(const Entity& e) {
    return {
        e.x + e.hurtbox_offsets.x,
        e.y + e.z + e.hurtbox_offsets.y,
        e.hurtbox_offsets.w,
        e.hurtbox_offsets.h
    };
}

SDL_FRect entity_get_world_hitbox(const Entity& e) {
    return {
        e.x + e.hitbox_offsets.x,
        e.y + e.z + e.hitbox_offsets.y,
        e.hitbox_offsets.w,
        e.hitbox_offsets.h
    };
}

void entity_draw(SDL_Renderer* r, const Entity& e, const Game* g) {
    assert(g != nullptr);

    const Vec2<f32> drawing_coords = entity_offset_to_bottom_center(e);
    Vec2<f32> screen_coords = game_get_screen_coords(*g, drawing_coords);
    screen_coords.y += e.z; // for jumping

    const SDL_FlipMode flip = (e.dir == Direction::Left) ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;
    bool ok = sprite_draw_at_dst(
        *e.anim.sprite,
        r,
        {
            .x_dst        = screen_coords.x,
            .y_dst        = screen_coords.y,
            .row          = e.anim.frames.idx,
            .col          = e.anim.frames.frame_current,
            .flip         = flip,
            .opacity      = e.anim.fadeout.perc_visible_curr,
            .rotation_deg = e.anim.rotation.deg_curr,
        }
    );
    if (!ok) SDL_Log("Failed to draw enemy sprite! SDL err: %s\n", SDL_GetError());

    Vec2<f32> world_coords = {e.x, e.y};
    draw_shadow(
        r,
        {
            .world_coords   = world_coords,
            .shadow_offsets = e.shadow_offsets,
            .g              = *g,
            .opacity        = e.anim.fadeout.perc_visible_curr
        }
    );

    // drawing debug *box
    {
        // this is so that both hurtbox and hitbox go along with the player when he jumps
        world_coords.y += e.z;

        if (settings.show_collision_boxes) draw_collision_box(r, world_coords, e.collision_box_offsets, *g);
        if (settings.show_hurtboxes) draw_hurtbox(r, world_coords, e.hurtbox_offsets, *g);
        if (settings.show_hitboxes) draw_hitbox(r, world_coords, e.hitbox_offsets, *g);
        if (settings.show_sprite_debug) {
            SDL_FRect sprite_bounds_screen = {
                screen_coords.x,
                screen_coords.y,
                e.sprite_frame_w,
                e.sprite_frame_h
            };
            _draw_box(r, sprite_bounds_screen, {255, 255, 0, 200}, {255, 255, 0, 50});
            
            // Draw entity position
            draw_point(r, {world_coords, *g, {255, 0, 255, 255}});
        }
    }
}

void entity_draw_knife(SDL_Renderer* r, const Entity& e, Game* g) {
    assert(g != nullptr);

    const Vec2<f32> drawing_coords = entity_offset_to_bottom_center(e);
    Vec2<f32> screen_coords = game_get_screen_coords(*g, drawing_coords);
    screen_coords.y += e.z; // for jumping

    const SDL_FlipMode flip = (e.dir == Direction::Left) ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;

    const Sprite* s = &g->sprite_knife_player;
    if (e.type == Entity_Type::Enemy) s = &g->sprite_knife_enemy;
    bool ok = sprite_draw_at_dst(
        *s,
        r,
        {
            .x_dst                         = screen_coords.x,
            .y_dst                         = screen_coords.y,
            .row                           = e.anim.frames.idx,
            .col                           = e.anim.frames.frame_current,
            .flip                          = flip,
            .opacity                       = e.anim.fadeout.perc_visible_curr,
            .center_of_rotation_offsets    = &game_get_mutable_entity_by_handle(*g, e.handle)->rotation_center_offsets,
            .return_on_failed_range_checks = true,
        }
    );
    if (!ok) SDL_Log("Failed to draw enemy sprite! SDL err: %s\n", SDL_GetError());
}

// returns whether the movement was in bounds or not
bool entity_movement_handle_collisions_and_pos_change(Entity& e, const Game* g, Collide_Opts opts) {
    assert(g != nullptr);

    f32 x_old = e.x;
    f32 y_old = e.y;
    e.x += e.x_vel * g->dt;
    e.y += e.y_vel * g->dt;

    bool in_bounds = true;
    SDL_FRect entity_collision_box = entity_get_world_collision_box(e);

    if (e.type == Entity_Type::Player) {
        for (const auto& box : level_info_get_collision_boxes(g->curr_level_info)) {
            if (SDL_HasRectIntersectionFloat(&box, &entity_collision_box)) {
                in_bounds = false;
                break;
            }
        }
    }

    if (e.type == Entity_Type::Enemy) {
        const auto& box = level_info_get_collision_box(g->curr_level_info, Border::Top);
        if (SDL_HasRectIntersectionFloat(&box, &entity_collision_box)) {
            in_bounds = false;
        }
    }

    if (e.type == Entity_Type::Collectible) {
        const auto& box_right = level_info_get_collision_box(g->curr_level_info, Border::Right);
        const auto& box_left = level_info_get_collision_box(g->curr_level_info, Border::Left);
        if (SDL_HasRectIntersectionFloat(&box_right, &entity_collision_box)
            || SDL_HasRectIntersectionFloat(&box_left, &entity_collision_box)) {
            in_bounds = false;
        }
    }

    if (opts.collide_with_walls) {
        for (const auto& box : level_info_get_collision_boxes(g->curr_level_info)) {
            if (SDL_HasRectIntersectionFloat(&box, &entity_collision_box)) {
                in_bounds = false;
                break;
            }
        }
    }

    if (in_bounds) {
        for (const auto& e_other : g->entities) {
            if (e.handle == e_other.handle) continue;
            auto skip = false;
            for (auto dont_with_type : opts.dont_collide_with) {
                if (e_other.type == dont_with_type) {
                    skip = true;
                    break;
                }
            }
            if (skip) continue;

            const auto& e_box = entity_get_world_collision_box(e_other);
            if (SDL_HasRectIntersectionFloat(&e_box, &entity_collision_box)) {
                in_bounds = false;
                break;
            }
        }
    }

    if (opts.reset_position_on_wall_impact && !in_bounds) {
        e.x = x_old;
        e.y = y_old;
    }

    return in_bounds;
}

Vec2<f32> calc_world_coordinates_of_slot(Vec2<f32> player_world_pos, const Player_Attack_Slots& slots, Slot slot) {
    switch (slot) {
        case Slot::None: {
            unreachable("coordinates can only be calculated for a valid slot");
            break;
        }

        case Slot::Top_Left: {
            return player_world_pos + slots.offset_top_left;
        }

        case Slot::Top_Right: {
            return player_world_pos + slots.offset_top_right;
        }

        case Slot::Bottom_Left: {
            return player_world_pos + slots.offset_bottom_left;
        }

        case Slot::Bottom_Right: {
            return player_world_pos + slots.offset_bottom_right;
        }
    }

    unreachable("just to make the compiler happy");
    return {};
}

Slot find_empty_slot(const Player_Attack_Slots& slots) {
    auto result = Slot::None;

    if (slots.top_right_free) {
        result = Slot::Top_Right;
    }
    else if (slots.bottom_right_free) {
        result = Slot::Bottom_Right;
    }
    else if (slots.top_left_free) {
        result = Slot::Top_Left;
    }
    else if (slots.bottom_left_free) {
        result = Slot::Bottom_Left;
    }

    return result;
}

Vec2<f32> claim_slot_position(Game& game, Slot slot) {
    Entity& player = game_get_player_mutable(game);
    const auto& slots = player.extra_player.slots;
    const Vec2<f32> player_pos = {player.x, player.y};

    switch (slot) {
        case Slot::None: {
            unreachable("only a valid position can be claimed");
        } break;

        case Slot::Top_Left: {
            if (slots.top_left_free) {
                player.extra_player.slots.top_left_free = false;
                return calc_world_coordinates_of_slot(player_pos, slots, slot);
            }
        } break;

        case Slot::Top_Right: {
            if (slots.top_right_free) {
                player.extra_player.slots.top_right_free = false;
                return calc_world_coordinates_of_slot(player_pos, slots, slot);
            }
        } break;

        case Slot::Bottom_Left: {
            if (slots.bottom_left_free) {
                player.extra_player.slots.bottom_left_free = false;
                return calc_world_coordinates_of_slot(player_pos, slots, slot);
            }
        } break;

        case Slot::Bottom_Right: {
            if (slots.bottom_right_free) {
                player.extra_player.slots.bottom_right_free = false;
                return calc_world_coordinates_of_slot(player_pos, slots, slot);
            }
        } break;
    }

    unreachable("just to make the compiler happy");
    return {};
}

void return_claimed_slot(Game& game, Slot slot) {
    Entity& player = game_get_player_mutable(game);

    switch (slot) {
        case Slot::None: {
            unreachable("only a valid position can be returned");
            break;
        }

        case Slot::Top_Left: {
            player.extra_player.slots.top_left_free = true;
            break;
        }

        case Slot::Top_Right: {
            player.extra_player.slots.top_right_free = true;
            break;
        }

        case Slot::Bottom_Left: {
            player.extra_player.slots.bottom_left_free = true;
            break;
        }

        case Slot::Bottom_Right: {
            player.extra_player.slots.bottom_right_free = true;
            break;
        }
    }
}

void entity_handle_rotating_hurtbox(Entity& e) {
    if (e.dir != e.dir_prev) {
        if ((e.dir_prev == Direction::Right && e.dir == Direction::Left) ||
            (e.dir_prev == Direction::Left  && e.dir == Direction::Right)) {
            e.hurtbox_offsets.x = -e.hurtbox_offsets.x - e.hurtbox_offsets.w;
        }
    }

    e.dir_prev = e.dir;
}

Entity* entity_pickup_collectible(const Entity& e, Game& g) {
    for (auto& collectible : g.entities) {
        if (collectible.type != Entity_Type::Collectible) continue;
        if (!collectible.extra_collectible.pickupable) continue;

        const auto& collision_box_e = entity_get_world_collision_box(e);
        const auto& collision_box_collectible = entity_get_world_collision_box(collectible);
        if (SDL_HasRectIntersectionFloat(&collision_box_collectible, &collision_box_e)) {
            return &collectible;
        }
    }
    return nullptr;
}
