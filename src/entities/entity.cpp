#include "entity.h"

void start_animation(Entity& e, u32 anim_idx, bool should_loop, u64 frame_time) {
    e.idx_anim          = anim_idx;
    e.current_frame     = 0;
    e.animation_playing = true;
    e.animation_loop    = should_loop;
    e.frame_duration_ms = frame_time;
    e.last_frame_time   = SDL_GetTicks();
}

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
