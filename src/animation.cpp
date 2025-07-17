#include <cassert>

#include "animation.h"

void animation_start(Animation& a, Anim_Start_Opts opts) {
    assert(a.sprite != nullptr);

    a.frames.idx               = opts.anim_idx;
    a.frames.frame_current     = 0;
    a.frames.frame_count       = a.sprite->frames_in_each_row[opts.anim_idx];
    a.playing                  = true;
    a.frames.looping           = opts.looping;
    a.frames.frame_duration_ms = opts.frame_duration_ms;
    a.frames.last_frame_time   = SDL_GetTicks();
    a.fadeout                  = opts.fadeout;
}

bool animation_is_finished(const Animation& a) {
    assert(a.sprite != nullptr);

    if (!a.playing) return true;
    if (a.frames.looping) return false;
    return a.frames.frame_current >= a.frames.frame_count - 1;
}

void animation_update(Animation& a) {
    assert(a.sprite != nullptr);

    if (!a.playing) return;

    u64 current_time = SDL_GetTicks();
    if (current_time - a.frames.last_frame_time >= a.frames.frame_duration_ms) {
        a.frames.frame_current++;
        a.frames.last_frame_time = current_time;

        // Check if animation finished
        if (a.frames.frame_current >= a.frames.frame_count) {
            if (a.frames.looping) {
                a.frames.frame_current = 0; // Loop back to start
            } else {
                a.playing = false;
                // loop the last frame until we start another animation
                a.frames.frame_current -= 1;
            }
        }
    }
}
