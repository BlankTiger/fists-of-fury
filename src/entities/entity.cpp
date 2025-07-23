#include <cassert>

#include "entity.h"
#include "../settings.h"
#include "../draw.h"
#include "../utils.h"

Vec2<f32> entity_offset_to_bottom_center(const Entity& e) {
    return {e.x - e.sprite_frame_w / 2, e.y - e.sprite_frame_h};
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
        e.y + e.hurtbox_offsets.y,
        e.hurtbox_offsets.w,
        e.hurtbox_offsets.h
    };
}

SDL_FRect entity_get_world_hitbox(const Entity& e) {
    return {
        e.x + e.hitbox_offsets.x,
        e.y + e.hitbox_offsets.y,
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
            .x_dst   = screen_coords.x,
            .y_dst   = screen_coords.y,
            .row     = e.anim.frames.idx,
            .col     = e.anim.frames.frame_current,
            .flip    = flip,
            .opacity = e.anim.fadeout.perc_visible_curr
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
        if (settings.show_collision_boxes) draw_collision_box(r, world_coords, e.collision_box_offsets, *g);

        // this is so that both hurtbox and hitbox go along with the player when he jumps
        world_coords.y += e.z;
        if (settings.show_hurtboxes) draw_hurtbox(r, world_coords, e.hurtbox_offsets, *g);

        if (settings.show_hitboxes) draw_hitbox(r, world_coords, e.hitbox_offsets, *g);
    }
}

void entity_movement_handle_collisions_and_pos_change(Entity& e, const Game* g, Collide_Opts opts) {
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

    if (in_bounds) {
        for (const auto& e_other : g->entities) {
            if (&e == &e_other) continue;
            auto skip = false;
            for (auto dont_with_type : opts.dont_collide_with) {
                if (e_other.type == dont_with_type) skip = true;
            }
            if (skip) continue;

            const auto& e_box = entity_get_world_collision_box(e_other);
            if (SDL_HasRectIntersectionFloat(&e_box, &entity_collision_box)) {
                in_bounds = false;
                break;
            }
        }
    }

    if (!in_bounds) {
        e.x = x_old;
        e.y = y_old;
    }
}

Vec2<f32> claim_slot_position(Game& game, Slot slot) {
    Entity& player = game_get_player_mutable(game);

    switch (slot) {
        case Slot::None: {
            unreachable("only a valid position can be claimed");
            break;
        }

        case Slot::Top_Left: {
            if (player.extra_player.slots.top_left_free)
                player.extra_player.slots.top_left_free = false;
            break;
        }

        case Slot::Top_Right: {
            if (player.extra_player.slots.top_right_free)
                player.extra_player.slots.top_right_free = false;
            break;
        }

        case Slot::Bottom_Left: {
            if (player.extra_player.slots.bottom_left_free)
                player.extra_player.slots.bottom_left_free = false;
            break;
        }

        case Slot::Bottom_Right: {
            if (player.extra_player.slots.bottom_right_free)
                player.extra_player.slots.bottom_right_free = false;
            break;
        }
    }

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
