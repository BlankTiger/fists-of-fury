#pragma once

#include "number_types.h"

#define internal static

const int SCREEN_WIDTH  = 100;
const int SCREEN_HEIGHT = 64;
const int WINDOW_WIDTH  = 1000;
const int WINDOW_HEIGHT = 640;

struct Settings {
    bool show_collision_boxes  = false;
    bool show_hurtboxes        = true;
    bool show_hitboxes         = true;
    bool dev_mode              = true;

    f32 collision_box_colors[4] = {255, 2, 0, 200};
    f32 hurtbox_colors[4]       = {2, 255, 0, 200};
    f32 hitbox_colors[4]        = {0, 2, 255, 200};

    f32 fps_max       = 144.0f;
    f32 gravity       = 0.00038f;
    f32 jump_velocity = -0.15f;
    f32 ground_level  = 0.0f;
};

extern Settings settings;
