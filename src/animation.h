#pragma once

#include <vector>

#include <SDL3/SDL.h>

#include "number_types.h"
#include "sprite.h"

struct Sprite_Frames {
    // idx of the animation (row in the sprite)
    u32 idx;

    // Current frame in the animation (col in the sprite)
    u32 frame_current;

    // Max frames for this animation before it is finished or it loops
    u32 frame_count;

    // Whether this animation should loop
    bool looping;
};

struct Fadeout {
    bool enabled            = false;
    bool looping            = false;
    f32  perc_per_sec       = 1.0f;
    f32  perc_visible_start = 1.0f;
    f32  perc_visible_end   = 0.0f;
    f32  perc_visible_curr  = 1.0f;
};

// should always contain positive numbers
// because when checking we take the absolute
// value of the deg_curr
struct Rotation_Range { f32 start, end; };

struct Rotation {
    bool                        enabled        = false;
    bool                        looping        = false;
    std::vector<Rotation_Range> finish_ranges  = {};
    f32                         deg_per_sec    = 30.0f;
    f32                         deg_curr       = 0.0f;
    f32                         deg_start      = 0.0f;
    u32                         rotations_min  = 1;
    u32                         rotations_curr = 0;
};

struct Animation {
    // sprite that will be animated
    const Sprite* sprite;

    f32 accumulated_time_ms;

    // Milliseconds per frame
    u64 frame_duration_ms;
    
    // this overrides all of the underyling looping variables for all systems
    // if set to true, otherwise the underyling systems choose if they loop
    bool          looping = false;
    Sprite_Frames frames;
    Fadeout       fadeout;
    Rotation      rotation;
};

struct Anim_Start_Opts {
    u32      anim_idx;
    u64      frame_duration_ms = 100;
    Fadeout  fadeout           = {};
    Rotation rotation          = {};
    bool     looping           = false;
};

void animation_start(Animation& a, Anim_Start_Opts opts);
void animation_update(Animation& a, u64 dt, u64 real_dt);
bool animation_is_finished(const Animation& a);
