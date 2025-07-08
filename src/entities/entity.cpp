#include "entity.h"

void start_animation(Entity& e, u32 anim_idx, bool should_loop, u64 frame_time) {
    e.idx_anim          = anim_idx;
    e.current_frame     = 0;
    e.animation_playing = true;
    e.animation_loop    = should_loop;
    e.frame_duration_ms = frame_time;
    e.last_frame_time   = SDL_GetTicks();
}
