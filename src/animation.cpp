#include <cassert>
#include <cmath>

#include "animation.h"

void animation_start(Animation& a, Anim_Start_Opts opts) {
    assert(a.sprite != nullptr);

    a.frames.idx           = opts.anim_idx;
    a.frames.frame_current = 0;
    a.frames.frame_count   = a.sprite->frames_in_each_row[opts.anim_idx];
    a.frames.looping       = opts.looping;
    a.frame_duration_ms    = opts.frame_duration_ms;
    a.accumulated_time_ms  = 0;
    a.fadeout              = opts.fadeout;
    a.rotation             = opts.rotation;
}

static bool fadeout_finished(const Fadeout& f) {
    if (!f.enabled) return true;
    return f.perc_visible_curr <= f.perc_visible_end;
}

static bool deg_in_range(f32 deg, const Rotation_Range& range) {
    f32 abs_deg = std::abs(deg);
    return abs_deg >= range.start && abs_deg <= range.end;
}

static bool rotation_finished(const Rotation& r) {
    if (!r.enabled) return true;
    if (r.rotations_curr <= r.rotations_min) return false;

    f32 deg = std::fmod(r.deg_curr, 360);
    for (const auto& range : r.finish_ranges) {
        if (deg_in_range(deg, range)) return true;
    }

    return false;
}

static bool any_looping(const Animation& a) {
    return a.looping
        || a.frames.looping
        || a.fadeout.looping
        || a.rotation.looping;
}

bool animation_is_finished(const Animation& a) {
    assert(a.sprite != nullptr);

    if (any_looping(a)) return false;
    if (!fadeout_finished(a.fadeout)) return false;
    if (!rotation_finished(a.rotation)) return false;

    // For single-frame animations, check if enough time has passed
    if (a.frames.frame_count == 1) {
        return a.accumulated_time_ms >= a.frame_duration_ms;
    }

    return a.frames.frame_current >= a.frames.frame_count - 1;
}

void animation_update(Animation& a, u64 dt, u64 real_dt) {
    assert(a.sprite != nullptr);

    a.accumulated_time_ms += dt;

    if (a.accumulated_time_ms >= a.frame_duration_ms && a.frames.frame_count != 1) {
        a.frames.frame_current++;
        a.accumulated_time_ms -= a.frame_duration_ms;

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
        const auto perc_to_fade = a.fadeout.perc_per_sec * real_dt / 1000.0f;
        a.fadeout.perc_visible_curr -= perc_to_fade;
        const auto faded = a.fadeout.perc_visible_curr <= a.fadeout.perc_visible_end;

        if (a.fadeout.looping && faded) {
            a.fadeout.perc_visible_curr = a.fadeout.perc_visible_start;
        } else if (faded) {
            a.fadeout.perc_visible_curr = 0.0f;
        }
    }

    if (a.rotation.enabled) {
        a.rotation.deg_curr += a.rotation.deg_per_sec * dt / 1000.0f;
        a.rotation.rotations_curr = (u32)((a.rotation.deg_curr - a.rotation.deg_start) / 360.0f);
    }
}
