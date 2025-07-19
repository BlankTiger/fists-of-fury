#include <cassert>

#include "entity.h"
#include "../settings.h"
#include "../draw.h"

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
