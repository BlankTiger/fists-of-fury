#include <cassert>

#include "animation.h"

void animation_start(Animation& a, Anim_Start_Opts opts) {
    assert(a.sprite != nullptr);

    a.idx               = opts.anim_idx;
    a.frame_current     = 0;
    a.frame_count       = a.sprite->frames_in_each_row[opts.anim_idx];
    a.playing           = true;
    a.looping           = opts.looping;
    a.frame_duration_ms = opts.frame_duration_ms;
    a.last_frame_time   = SDL_GetTicks();
}

bool animation_is_finished(const Animation& a) {
    assert(a.sprite != nullptr);

    if (!a.playing) return true;
    if (a.looping) return false;
    return a.frame_current >= a.frame_count - 1;
}

void animation_update(Animation& a) {
    assert(a.sprite != nullptr);

    if (!a.playing) return;

    u64 current_time = SDL_GetTicks();
    if (current_time - a.last_frame_time >= a.frame_duration_ms) {
        a.frame_current++;
        a.last_frame_time = current_time;

        // Check if animation finished
        if (a.frame_current >= a.frame_count) {
            if (a.looping) {
                a.frame_current = 0; // Loop back to start
            } else {
                a.playing = false;
                // loop the last frame until we start another animation
                a.frame_current -= 1;
            }
        }
    }
}
