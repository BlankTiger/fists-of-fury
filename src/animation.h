#pragma once

#include "number_types.h"
#include "sprite.h"
#include <SDL3/SDL.h>

struct Animation {
    // sprite that will be animated
    const Sprite* sprite;

    // idx of the animation (row in the sprite)
    u32 idx;

    // Current frame in the animation (col in the sprite)
    u32 frame_current;

    // Max frames for this animation before it is finished or it loops
    u32 frame_count;

    // Whether the animation is currently playing
    bool playing;

    // Whether this animation should loop
    bool looping;

    // When the last frame was shown
    u64 last_frame_time;

    // Milliseconds per frame
    u64 frame_duration_ms;

    // TODO: add subsystems to this:
    // - something like fading for example
};

void animation_start(Animation& a, u32 anim_idx, bool looping, u64 frame_duration_ms = 100);
void animation_update(Animation& a);
bool animation_is_finished(const Animation& a);
