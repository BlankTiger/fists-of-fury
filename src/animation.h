#pragma once

#include "number_types.h"
#include "sprite.h"
#include <SDL3/SDL.h>

struct Sprite_Frames {
    // idx of the animation (row in the sprite)
    u32 idx;

    // Current frame in the animation (col in the sprite)
    u32 frame_current;

    // Max frames for this animation before it is finished or it loops
    u32 frame_count;

    // Whether this animation should loop
    bool looping;

    // When the last frame was shown
    u64 last_frame_time;

    // Milliseconds per frame
    u64 frame_duration_ms;
};

struct Fadeout {
    bool looping      = false;
    f32  perc_per_sec = 20;
};

struct Animation {
    // sprite that will be animated
    const Sprite* sprite;

    // Whether the animation is currently playing
    bool playing;

    // this overrides all of the underyling looping variables for all systems
    // if set to true, otherwise the underyling systems choose if they loop
    bool          looping = false;
    Sprite_Frames frames;
    Fadeout       fadeout;
};

struct Anim_Start_Opts {
    u32     anim_idx;
    bool    looping           = false;
    u64     frame_duration_ms = 100;
    Fadeout fadeout           = {};
};

void animation_start(Animation& a, Anim_Start_Opts opts);
void animation_update(Animation& a);
bool animation_is_finished(const Animation& a);
