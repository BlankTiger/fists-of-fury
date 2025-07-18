#include <cassert>

#include "animation.h"

void animation_start(Animation& a, Anim_Start_Opts opts) {
    assert(a.sprite != nullptr);

    a.frames.idx           = opts.anim_idx;
    a.frames.frame_current = 0;
    a.frames.frame_count   = a.sprite->frames_in_each_row[opts.anim_idx];
    a.frames.looping       = opts.looping;
    a.frame_duration_ms    = opts.frame_duration_ms;
    a.last_frame_time      = SDL_GetTicks();
    a.fadeout              = opts.fadeout;
}

static bool fadeout_finished(const Fadeout& f) {
    if (!f.enabled) return true;
    return f.perc_visible_curr <= f.perc_visible_end;
}

static bool any_looping(const Animation& a) {
    return a.looping || a.frames.looping || a.fadeout.looping;
}

bool animation_is_finished(const Animation& a) {
    assert(a.sprite != nullptr);

    if (any_looping(a)) return false;
    if (!fadeout_finished(a.fadeout)) return false;
    return a.frames.frame_current >= a.frames.frame_count - 1;
}

void animation_update(Animation& a) {
    assert(a.sprite != nullptr);

    u64 current_time = SDL_GetTicks();
    const auto dt = current_time - a.last_frame_time;
    if (dt >= a.frame_duration_ms) {
        a.frames.frame_current++;
        a.last_frame_time = current_time;

        // Check if sprite animation finished
        if (a.frames.frame_current >= a.frames.frame_count) {
            if (a.frames.looping) {
                a.frames.frame_current = 0; // Loop back to start
            } else {
                // loop the last frame until we start another animation
                a.frames.frame_current -= 1;
            }
        }

    }

    if (a.fadeout.enabled) {
        // current_time - a.last_frame_time = dt (ms)
        // dt (s) = dt / 1000
        // a.fadeout.perc_per_sec*dt
        const auto perc_to_fade = a.fadeout.perc_per_sec * dt / 1000.0f;
        a.fadeout.perc_visible_curr -= perc_to_fade;
        const auto faded = a.fadeout.perc_visible_curr <= a.fadeout.perc_visible_end;
        if (a.fadeout.looping && faded) {
            a.fadeout.perc_visible_curr = a.fadeout.perc_visible_start;
        }
        else if (faded) {
            a.fadeout.perc_visible_curr = 0.0f;
        }
    }
}
