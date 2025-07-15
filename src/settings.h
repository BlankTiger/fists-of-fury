#pragma once

#include <array>
#include "number_types.h"

const int SCREEN_WIDTH  = 100;
const int SCREEN_HEIGHT = 64;
const int WINDOW_WIDTH  = 1000;
const int WINDOW_HEIGHT = 640;

struct Settings {
    bool show_collision_boxes  = false;
    bool show_hurtboxes        = true;
    bool show_hitboxes         = true;
    bool dev_mode              = true;

    std::array<f32, 4> collision_box_colors_border = {255,      2,        0,        200};
    std::array<f32, 4> collision_box_colors_fill   = {255/2.0f, 2/2.0f,   0/2.0f,   200/2.0f};

    std::array<f32, 4> hurtbox_colors_border       = {2,        255,      0,        200};
    std::array<f32, 4> hurtbox_colors_fill         = {2/2.0f,   255/2.0f, 0/2.0f,   200/2.0f};

    std::array<f32, 4> hitbox_colors_border        = {0,        2,        255,      200};
    std::array<f32, 4> hitbox_colors_fill          = {0/2.0f,   2/2.0f,   255/2.0f, 200/2.0f};

    f32 fps_max       = 144.0f;
    f32 gravity       = 0.00038f;
    f32 jump_velocity = -0.15f;
    f32 ground_level  = 0.0f;
};

extern Settings settings;
